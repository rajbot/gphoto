/*
 *	File: main.c
 *
 *	Copyright (C) 1998 Ugo Paternostro <paterno@dsi.unifi.it>
 *
 *	This file is part of the dc20ctrl package. The complete package can be
 *	downloaded from:
 *	    http://aguirre.dsi.unifi.it/~paterno/binaries/dc20ctrl.tar.gz
 *
 *	This package is derived from the dc20 package, built by Karl Hakimian
 *	<hakimian@aha.com> that you can find it at ftp.eecs.wsu.edu in the
 *	/pub/hakimian directory. The complete URL is:
 *	    ftp://ftp.eecs.wsu.edu/pub/hakimian/dc20.tar.gz
 *
 *	This package also includes a sligthly modified version of the Comet to ppm
 *	conversion routine written by YOSHIDA Hideki <hideki@yk.rim.or.jp>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published 
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *	Main program for dc20ctrl: reads the options and take the actions.
 *
 *	$Id$
 */

#include <time.h>

#include "dc20.h"
#include "main.h"
#include "session.h"
#include "init_dc20.h"
#include "shoot.h"
#include "thumbs_to_file.h"
#include "pics_to_file.h"
#include "erase.h"
#include "change_res.h"
#include "convert_pic.h"
#include "comet_to_ppm.h"

#define BASE_OPTS	"ab:B:cefg:G:hH:iklL:n:opP:qrR:sS:tT:vVw"

#ifdef USE_JPEG
#define JPEG_OPTS	"jQ:"
#else
#define JPEG_OPTS
#endif /* USE_JPEG */

#ifdef USE_TIFF
#define TIFF_OPTS	"FC:"
#else
#define TIFF_OPTS
#endif /* USE_TIFF */

#ifdef USE_PNG
#define PNG_OPTS	"N"
#else
#define PNG_OPTS
#endif /* USE_PNG */

#define DC20CTRL_OPTS	BASE_OPTS JPEG_OPTS TIFF_OPTS PNG_OPTS

#ifndef DC20CTRL_TTY
#ifdef __FreeBSD__
#define DC20CTRL_TTY	"/dev/cuaa0"
#else
#define DC20CTRL_TTY	"/dev/ttyS0"
#endif /* __FreeBSD__ */
#endif /* DC20CTRL_TTY */

#define NAME_LEN		128

#define ACTION_INFO		0x01
#define ACTION_SHOOT	0x02
#define ACTION_THUMBS	0x04
#define ACTION_PICS		0x08
#define ACTION_ERASE	0x10
#define ACTION_LORES	0x20
#define ACTION_HIRES	0x40
#define ACTION_TGLRES	0x60

#define MASK_RES		0x60

int	 verbose = FALSE,
	 quiet = FALSE;

#ifdef USE_JPEG
int	 quality = 90;
#endif /* USE_JPEG */

#ifdef USE_TIFF
int	 tiff_compression = COMPRESSION_NONE,
	 tiff_predictor = 0;
#endif /* USE_TIFF */

/*
 *	Print usage
 */

static void usage(void)
{
	printf("usage: %s [-V]\n" \
		   "       %s [-b baud-rate] [-e [-f]] [-h|-l|-r] [-i]\n" \
		   "           [ [-p [-P pics-base-name]\n" \
		   "               [ {-w|-c} [-R red-factor] [-G green-factor] [-B blue-factor]\n" \
		   "                 [-S saturation] [-g gamma] [-L low-i] [-H high-i] [-a] [-k]\n" \
		   "               ]\n" \
		   "             ] [-t [-T thumbs-base-name]] [-n pictures]\n"
#ifdef USE_JPEG
		   "             [-j [-Q quality] ]\n"
#endif /* USE_JPEG */
#ifdef USE_TIFF
		   "             [-F [-C compression[:option]] ]\n"
#endif /* USE_TIFF */
#ifdef USE_PNG
		   "             [-N]\n"
#endif /* USE_PNG */
		   "           ] [-q|-v] [-s] [device-name]\n" \
		   "       %s -o [same options as -p, but -P] file-1 [... [file-16] ]\n" , __progname, __progname, __progname);
}

/*
 *	Select orientation
 */

static int get_orientation(char c)
{
	int	 result;

	switch (c) {
		case 'l':
		case 'L':
			result = ROT_LEFT;
			break;
		case 'r':
		case 'R':
			result = ROT_RIGHT;
			break;
		case 'b':
		case 'B':
			result = ROT_HEADDOWN;
			break;
		default:
			result = ROT_STRAIGHT;
			break;
	}

	return result;
}

/*
 *	Parse the pictures selection string
 */

static int parse_pics(char *string, int *orientation_mask)
{
	char	*pivot = string,
			*pivot2,
			*pivot3;
	int		 result = 0,
			 i,
			 first,
			 last,
			 orientation = ROT_STRAIGHT,
			 this_orientation;

	while (string) {
		strsep(&pivot, ",");
		/*
		 *	string points to the next piece to be parsed, up to the comma.
		 */
		pivot2 = string;
		strsep(&pivot2, ":");
		if (*string < '0' || *string > '9') {
			/*
			 *	"orientation:range"
			 */
			orientation = get_orientation(*string);
			string = pivot2;
		} else {
			/*
			 *	"range:orientation" or simply "range", don't care
			 */
			pivot2 = string;
		}
		this_orientation = orientation; /* sets default orientation */
		strsep(&pivot2, "-");
		first = strtol(string, &pivot3, 10);
		if (first < 1 || first > 16) {
			if (!quiet) fprintf(stderr, "%s: parse_pics: error: out of range %d\n", __progname, first);
			return -1;
		}
		if (pivot2) {
			if (*pivot3) {
				if (!quiet) fprintf(stderr, "%s: parse_pics: error: extraneous characters '%s' in %d%s-%s\n", __progname, pivot3, first, pivot3, pivot2);
				return -1;
			}
			/*
			 *	first-number
			 */
			last = strtol(pivot2, &pivot3, 10);
			if (last < 1 || last > 16) {
				if (!quiet) fprintf(stderr, "%s: parse_pics: error: out of range %d\n", __progname, last);
				return -1;
			}
		} else {
			last = first;
		}

		if (*pivot3) {
			/*
			 *	"numberorientation"
			 */
			this_orientation = get_orientation(*pivot3);
		}

		if (last < first) {
			i = last;
			last = first;
			first = i;
		}

		for (i=first; i<=last; i++) {
			result |= 1 << (i-1);
			*orientation_mask |= this_orientation << (2 * (i-1));
		}

		string = pivot;
	}

	return result;
}

/*
 *	Main program: parse switches and take actions
 */

void main(int argc, char *argv[])
{
	int			 curopt,
				 actions = 0,
				 force = FALSE,
				 pic_mask = 0xFFFF,
				 save_format = SAVE_RAW,
				 orientation_mask = 0,
				 offline = FALSE,
				 keep = FALSE,
				 to_be_converted,
				 tfd,
				 i,
				 session = 0;
	char		*tty_name = DC20CTRL_TTY,
#ifdef USE_TIFF
				*compr_opt,
#endif /* USE_TIFF */
				 name_template[NAME_LEN],
				 pics_name[NAME_LEN],
				 thumbs_name[NAME_LEN],
				*pics_pre = NULL,
				*thumbs_pre = NULL,
				*pics_list[16],
				**list;
/* PSF */
	speed_t		 tty_baud = B115200;
	Dc20Info	*dc20_info;
	time_t		 clock;

	/*
	 *	Parse arguments
	 */

	while ((curopt = getopt(argc, argv, DC20CTRL_OPTS)) != EOF) {
		switch (curopt) {
			case 'a':
				save_format |= SAVE_ADJASPECT;
				break;
			case 'b':
				switch (atoi(optarg)) {
					case 9600:
						tty_baud = B9600;
						break;
					case 19200:
						tty_baud = B19200;
						break;
					case 38400:
						tty_baud = B38400;
						break;
					case 57600:
						tty_baud = B57600;
						break;
					case 115200:
						tty_baud = B115200;
						break;
					default:
						if (verbose) printf("%s: unsupported baud rate %s, using default\n", __progname, optarg);
						break;
				}
				break;
			case 'B':
				bfactor = atof(optarg);
				break;
			case 'c':
			case 'w':
				if (save_format & SAVE_GREYSCALE || save_format & SAVE_24BITS) {
					fprintf(stderr, "%s: error: you may specify at most one between -c and -w switches\n", __progname);
					usage();
					exit(1);
				}
				save_format = (save_format & ~SAVE_FILES) | (curopt == 'c') ? SAVE_24BITS : SAVE_GREYSCALE;
				break;
#ifdef USE_TIFF
			case 'C':
				compr_opt = optarg;
				strsep(&compr_opt, ":");
				switch (*optarg) {
					case 'j':
					case 'J':
						tiff_compression = COMPRESSION_JPEG;
						if (compr_opt) {
							quality = atoi(compr_opt);
						}
						break;
					case 'p':
					case 'P':
						tiff_compression = COMPRESSION_PACKBITS;
						break;
					case 'l':
					case 'L':
						tiff_compression = COMPRESSION_LZW;
						if (compr_opt) {
							tiff_predictor = atoi(compr_opt);
						}
						break;
					case 'z':
					case 'Z':
					case 'd':
					case 'D':
						tiff_compression = COMPRESSION_DEFLATE;
						if (compr_opt) {
							tiff_predictor = atoi(compr_opt);
						}
						break;
					default:
						tiff_compression = COMPRESSION_NONE;
						break;
				}
				break;
#endif /* USE_TIFF */
			case 'e':
				actions |= ACTION_ERASE;
				break;
			case 'f':
				force = TRUE;
				break;
#ifdef USE_TIFF
			case 'F':
				save_format = (save_format & ~SAVE_FORMATS) | SAVE_TIFF;
				break;
#endif /* USE_TIFF */
			case 'g':
				gamma_value = atof(optarg);
				break;
			case 'G':
				gfactor = atof(optarg);
				break;
			case 'h':
			case 'l':
			case 'r':
				if (actions & MASK_RES) {
					fprintf(stderr, "%s: error: you may specify at most one between -h, -l and -r switches\n", __progname);
					usage();
					exit(1);
				}
				switch (curopt) {
					case 'h':
						actions |= ACTION_HIRES;
						break;
					case 'l':
						actions |= ACTION_LORES;
						break;
					case 'r':
						actions |= ACTION_TGLRES;
						break;
				}
				break;
			case 'H':
				high_i = atoi(optarg);
				break;
			case 'i':
				actions |= ACTION_INFO;
				break;
#ifdef USE_JPEG
			case 'j':
				save_format = (save_format & ~SAVE_FORMATS) | SAVE_JPEG;
				break;
#endif /* USE_JPEG */
			case 'k':
				keep = TRUE;
				break;
			case 'L':
				low_i = atoi(optarg);
				break;
			case 'n':
				if ( (pic_mask = parse_pics(optarg, &orientation_mask)) == -1 ) {
					if (!quiet) fprintf(stderr, "%s: error: wrong range used\n", __progname);
					exit(1);
				}
				break;
#ifdef USE_PNG
			case 'N':
				save_format = (save_format & ~SAVE_FORMATS) | SAVE_PNG;
				break;
#endif /* USE_PNG */
			case 'o':
				offline = TRUE;
				break;
			case 'p':
				actions |= ACTION_PICS;
				break;
			case 'P':
				pics_pre = optarg;
				break;
			case 'q':
			case 'v':
				if (verbose || quiet) {
					fprintf(stderr, "%s: error: you may specify only once -q or -v\n", __progname);
					exit(1);
				}
				if (curopt == 'q')
					quiet = TRUE;
				else
					verbose = TRUE;
				break;
#ifdef USE_JPEG
			case 'Q':
				quality = atoi(optarg);
				break;
#endif /* USE_JPEG */
			case 'R':
				rfactor = atof(optarg);
				break;
			case 's':
				actions |= ACTION_SHOOT;
				break;
			case 'S':
				saturation = atof(optarg);
				break;
			case 't':
				actions |= ACTION_THUMBS;
				break;
			case 'T':
				thumbs_pre = optarg;
				break;
			case 'V':
				printf("%s version %d.%d (" __DATE__ ")\n" \
					   "Copyright (C) 1998 Ugo Paternostro <paterno@dsi.unifi.it>\n" \
					   "%s comes with ABSOLUTELY NO WARRANTY: this is free software,\n" \
					   "and you are welcome to redistribute it under certain conditions;\n" \
					   "see COPYING for details\n", __progname, VERSION, REVISION, __progname);
				exit(0);
				break;
			case '?':
			default:
				usage();
				exit(1);
				break;
		}
	}
	argc -= optind;
	argv += optind;

	if (!offline) {
		if (argc > 1) {
			usage();
			exit(1);
		}

		if (argc == 1)
			tty_name = argv[0];

		/*
		 *	Get session number from .dc20ctrlrc
		 */

		if ((session = get_session()) == -1) {
			fprintf(stderr, "%s: error: get_session returned -1\n", __progname);
			exit(1);
		}
		
		/*
		 *	Setup pictures and thumbnails names
		 */
	
		clock = time(NULL);

		if (pics_pre) {
			sprintf(pics_name, "%s_%%d.%%s", pics_pre);
		} else {
			strftime(name_template, NAME_LEN, "%%s_%Y_%m_%d_%%d_%%%%d.%%%%s", localtime(&clock));
			sprintf(pics_name, name_template, "pic", session);
		}

		if (thumbs_pre) {
			sprintf(thumbs_name, "%s_%%d", thumbs_pre);
		} else {
			strftime(name_template, NAME_LEN, "%%s_%Y_%m_%d_%%d_%%%%d", localtime(&clock));
			sprintf(thumbs_name, name_template, "thumb", session);
		}

		if (actions == 0) {
			if (verbose) printf("%s: warning: no action specified, quering information\n", __progname);
			actions = ACTION_INFO;
		}
	
		if (verbose && force && !(actions & ACTION_ERASE))
			printf("%s: warning: ignoring -f\n", __progname);
	
		if (verbose && save_format == (SAVE_RAW | SAVE_ADJASPECT))
			printf("%s: warning: ignoring -a\n", __progname);
	
		/*
		 *	Let's start!
		 */
	
		if ((tfd = init_dc20(tty_name, tty_baud)) == -1) {
			if (!quiet) fprintf(stderr, "%s: error: could not initialize the camera\n", __progname);
			exit(1);
		}
	
		if ((dc20_info = get_info(tfd)) == NULL) {
			if (!quiet) fprintf(stderr, "%s: error: could not get info\n", __progname);
			close_dc20(tfd);
			exit(1);
		}
	
		/*
		 *	Do actions exactly in this order, so they can be done safely
		 */
	
		if (actions & ACTION_INFO) {
			printf("\nDC20 information:\n~~~~~~~~~~~~~~~~~\n\n");
			printf("Model...........: DC%x\n", dc20_info->model);
			printf("Firmware version: %d.%d\n", dc20_info->ver_major, dc20_info->ver_minor);
			printf("Pictures........: %d/%d\n", dc20_info->pic_taken, dc20_info->pic_taken + dc20_info->pic_left);
			printf("Resolution......: %s\n", dc20_info->flags.low_res ? "low" : "high");
			printf("Battery state...: %s\n", dc20_info->flags.low_batt ? "low" : "good");
		}
	
		if (actions & ACTION_SHOOT) {
			if (dc20_info->pic_left) {
				shoot(tfd);
				dc20_info->pic_taken++;
				dc20_info->pic_left--;
			} else
				if (!quiet) fprintf(stderr, "%s: error: not enough memory\n", __progname);
		}
	
		/*
		 *	Fix the pictures mask to match the pictures in the camera
		 */
	
		if (dc20_info->flags.low_res) pic_mask &= 0x00FF;
	
		pic_mask &= ((1 << dc20_info->pic_taken) - 1);
	
		if (actions & ACTION_THUMBS) {
			printf("\n");
			thumbs_to_file(tfd, pic_mask, thumbs_name, save_format, orientation_mask);
		}
	
		if (actions & ACTION_PICS) {
			printf("\n");
			if (pics_to_file(tfd, pic_mask, dc20_info->flags.low_res, pics_name, pics_list) == -1) {
				if (!quiet) fprintf(stderr, "%s: error: could not download pictures\n", __progname);
				close_dc20(tfd);
				exit(1);
			}
		}
	
		if (actions & ACTION_ERASE) {
			if (!force) {
				printf("\nYou asked to delete all images from DC20 memory.\n\nAre you sure? (y/N) ");
				if ((force = getchar()) == 'y' || force == 'Y')
					force = TRUE;
				else
					force = FALSE;
			}
			if (force) {
				erase(tfd);
				dc20_info->pic_left += dc20_info->pic_taken;
				dc20_info->pic_taken = 0;
			}
		}
	
		if (actions & MASK_RES) {
			if (dc20_info->pic_taken) {
				if (!quiet) fprintf(stderr, "%s: error: cannot change resolution while camera memory is not empty\n", __progname);
			} else {
				unsigned char	 new_res;
	
				if ((actions & ACTION_TGLRES) == ACTION_TGLRES) {
					new_res = dc20_info->flags.low_res ? 0 : 1;
				} else {
					new_res = (actions & ACTION_LORES) ? 1 : 0;
				}
	
				if (dc20_info->flags.low_res == new_res) {
					if (verbose) {
						printf("%s: warning: the camera is yet in %s resolution mode\n", __progname, dc20_info->flags.low_res ? "low" : "high");
					}
				} else {
					change_res(tfd, new_res);
				}
			}
		}

		close_dc20(tfd);

		if (!quiet) printf("\nNow you may safely turn off the camera\n");

		if (put_session(++session) == -1) {
			if (!quiet) fprintf(stderr, "%s: error: put_session returnet -1\n", __progname);
			exit(1);
		}

		list = pics_list;
		to_be_converted = 16;
	} else {
		list = argv;
		if (argc > 16) {
			if (!quiet) printf("%s: warning: batch converting only the first 16 of %d pictures\n", __progname, argc);
			to_be_converted = 16;
		} else
			to_be_converted = argc;
	}

	if ( (save_format & SAVE_FILES) != SAVE_RAW) {
		/*
		 *	Convert pictures
		 */
		for (i = 0; i < to_be_converted; i++) {
			if (pic_mask & (1 << i) && list[i] != NULL) {
				convert_pic(list[i], save_format, (orientation_mask >> (i*2)) & ROT_MASK);
				if (!keep)
					unlink(list[i]);
				if (!offline)
					free(list[i]);
			}
		}
	}

	exit(0);
}
