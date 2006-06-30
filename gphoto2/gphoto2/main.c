/*
 * Copyright � 2002 Lutz M�ller <lutz@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include "actions.h"
#include "foreach.h"
#include "gphoto2-port-info-list.h"
#include "gphoto2-port-log.h"
#include "gphoto2-setting.h"
#include "gp-params.h"
#include "i18n.h"
#include "main.h"
#include "options.h"
#include "range.h"
#include "shell.h"

#ifdef HAVE_CDK
#  include "gphoto2-cmd-config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <locale.h>

#ifdef HAVE_POPT
#  include <popt.h>
/* POPT_TABLEEND is only defined from popt 1.6.1 */
# ifndef POPT_TABLEEND
#  define POPT_TABLEEND { NULL, '\0', 0, 0, 0, NULL, NULL }
# endif
#else
# error gphoto2 now REQUIRES the popt library!
#endif

#ifdef HAVE_RL
#  include <readline/readline.h>
#endif

#ifdef HAVE_PTHREAD
#  include <pthread.h>
#endif

#ifndef WIN32
#  include <signal.h>
#endif

#ifndef MAX
# define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define CR(result) {int r = (result); if (r < 0) return (r);}

int  glob_debug = -1;
char glob_cancel = 0;
int  glob_frames = 0;
int  glob_interval = 0;

GPParams p;


static struct {
	CameraFileType type;
	const char *prefix;
} PrefixTable[] = {
	{GP_FILE_TYPE_NORMAL, ""},
	{GP_FILE_TYPE_PREVIEW, "thumb_"},
	{GP_FILE_TYPE_RAW, "raw_"},
	{GP_FILE_TYPE_AUDIO, "audio_"},
	{GP_FILE_TYPE_METADATA, "meta_"},
	{0, NULL}
};

#undef  MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

static int
get_path_for_file (const char *folder, CameraFile *file, char **path)
{
	unsigned int i, l;
	int n;
	char *s, b[1024];
	const char *name, *prefix;
	CameraFileType type;
	time_t t;
	struct tm *tm;
	int hour12;

	if (!file || !path)
		return (GP_ERROR_BAD_PARAMETERS);

	*path = NULL;
	CR (gp_file_get_name (file, &name));
	CR (gp_file_get_mtime (file, &t));
	tm = localtime (&t);
	hour12 = tm->tm_hour % 12;
	if (hour12 == 0) {
		hour12 = 12;
	}

	/*
	 * If the user didn't specify a filename, use the original name 
	 * (and prefix).
	 */
	if (!p.filename || !strcmp (p.filename, "")) {
		CR (gp_file_get_type (file, &type));
		for (i = 0; PrefixTable[i].prefix; i++)
			if (PrefixTable[i].type == type)
				break;
		prefix = (PrefixTable[i].prefix ? PrefixTable[i].prefix :
						  "unknown_");
		*path = malloc (strlen (prefix) + strlen (name) + 1);
		if (!*path)
			return (GP_ERROR_NO_MEMORY);
		strcpy (*path, prefix);
		strcat (*path, name);
		return (GP_OK);
	}

	/* The user did specify a filename. Use it. */
	b[sizeof (b) - 1] = '\0';
	for (i = 0; i < strlen (p.filename); i++) {
		if (p.filename[i] == '%') {
			char padding = '0'; /* default padding character */
			int precision = 0;  /* default: no padding */
			i++;
			/* determine padding character */
			switch (p.filename[i]) {
			  /* case ' ':
			   * spaces are not supported everywhere, so we
			   * restrict ourselves to padding with zeros.
			   */
			case '0':
				padding = p.filename[i];
				precision = 1; /* do padding */
				i++;
				break;
			}
			/* determine padding width */
			if (isdigit((int)p.filename[i])) {
				char *cp;
				long int _prec;
				_prec = strtol(&p.filename[i],
					       &cp, 10);
				if (_prec < 1) 
					precision = 1;
				else if (_prec > 20)
					precision = 20;
				else
					precision = _prec;
				if (*cp != 'n') {
					/* make sure this is %n */
					gp_context_error (p.context,
						  _("Zero padding numbers "
						    "in file names is only "
						    "possible with %%n."));
					return GP_ERROR_BAD_PARAMETERS;
				}
				/* go to first non-digit character */
				i += (cp - &p.filename[i]);
			} else if (precision && ( p.filename[i] != 'n')) {
				gp_context_error (p.context,
					  _("You cannot use %%n "
					    "zero padding "
					    "without a "
					    "precision value!"
					    ));
				return GP_ERROR_BAD_PARAMETERS;
			}
			switch (p.filename[i]) {
			case 'n':

				/*
				 * Get the number of the file. This can only
				 * be done with persistent files!
				 */
				if (!folder) {
					gp_context_error (p.context, 
						_("You cannot use '%%n' "
						  "in combination with "
						  "non-persistent files!"));
					return (GP_ERROR_BAD_PARAMETERS);
				}
				n = gp_filesystem_number (p.camera->fs,
					folder, name, p.context);
				if (n < 0) {
					free (*path);
					*path = NULL;
					return (n);
				}
				if (precision > 1) {
					char padfmt[16];
					strcpy(padfmt, "%!.*i");
					padfmt[1] = padding;
					snprintf (b, sizeof (b), padfmt,
						  precision, n + 1);
				} else {
					snprintf (b, sizeof (b), "%i",
						  n + 1);
				}
				break;

			case 'C':
				/* Get the suffix of the original name */
				s = strrchr (name, '.');
				if (!s) {
					free (*path);
					*path = NULL;
					gp_context_error (p.context,
						_("The filename provided "
						  "by the camera ('%s') "
						  "does not contain a "
						  "suffix!"), name);
					return (GP_ERROR_BAD_PARAMETERS);
				}
				strncpy (b, s + 1, sizeof (b) - 1);
				break;

			case 'f':

				/* Get the file name without suffix */
				s = strrchr (name, '.');
				if (!s)
					strncpy (b, name, sizeof (b) - 1);
				else {
					l = MIN (sizeof (b) - 1, s - name);
					strncpy (b, name, l);
					b[l] = '\0';
				}
				break;

			case 'a':
			case 'A':
			case 'b':
			case 'B':
			case 'd':
			case 'H':
			case 'k':
			case 'I':
			case 'l':
			case 'j':
			case 'm':
			case 'M':
			case 'S':
			case 'y':
			case 'Y':
				{
					char fmt[3] = { '%', '\0', '\0' };
					fmt[1] = p.filename[i]; /* the letter of this 'case' */
					strftime(b, sizeof (b), fmt, tm);
					break;
				}
			case '%':
				strcpy (b, "%");
				break;
			default:
				free (*path);
				*path = NULL;
				gp_context_error (p.context,
					_("Invalid format '%s' (error at "
					  "position %i)."), p.filename,
					i + 1);
				return (GP_ERROR_BAD_PARAMETERS);
			}
		} else {
			b[0] = p.filename[i];
			b[1] = '\0';
		}

		s = *path ? realloc (*path, strlen (*path) + strlen (b) + 1) :
			    malloc (strlen (b) + 1);
		if (!s) {
			free (*path);
			*path = NULL;
			return (GP_ERROR_NO_MEMORY);
		}
		if (*path) {
			*path = s;
			strcat (*path, b);
		} else {
			*path = s;
			strcpy (*path, b);
		}
	}

	return (GP_OK);
}

int
save_camera_file_to_file (const char *folder, CameraFile *file)
{
	char *path = NULL, s[1024], c[1024];
	CameraFileType type;

	CR (gp_file_get_type (file, &type));

	CR (get_path_for_file (folder, file, &path));
	strncpy (s, path, sizeof (s) - 1);
	s[sizeof (s) - 1] = '\0';
	free (path);
	path = NULL;

        if ((p.flags & FLAGS_QUIET) == 0) {
                while ((p.flags & FLAGS_FORCE_OVERWRITE) == 0 &&
		       gp_system_is_file (s)) {
			do {
				putchar ('\007');
				printf (_("File %s exists. Overwrite? [y|n] "),
					s);
				fflush (stdout);
				if (NULL == fgets (c, sizeof (c) - 1, stdin))
					return GP_ERROR;
			} while ((c[0]!='y')&&(c[0]!='Y')&&
				 (c[0]!='n')&&(c[0]!='N'));

			if ((c[0]=='y') || (c[0]=='Y'))
				break;

			do { 
				printf (_("Specify new filename? [y|n] "));
				fflush (stdout); 
				if (NULL == fgets (c, sizeof (c) - 1, stdin))
					return GP_ERROR;
			} while ((c[0]!='y')&&(c[0]!='Y')&&
				 (c[0]!='n')&&(c[0]!='N'));

			if (!((c[0]=='y') || (c[0]=='Y')))
				return (GP_OK);

			printf (_("Enter new filename: "));
			fflush (stdout);
			if (NULL == fgets (s, sizeof (s) - 1, stdin))
				return GP_ERROR;
			s[strlen (s) - 1] = 0;
                }
                printf (_("Saving file as %s\n"), s);
		fflush (stdout);
        }
	CR (gp_file_save (file, s));

	return (GP_OK);
}

int
camera_file_exists (Camera *camera, GPContext *context, const char *folder,
		    const char *filename, CameraFileType type)
{
	CameraFileInfo info;
	CR (gp_camera_file_get_info (camera, folder, filename, &info,
				     context));
	switch (type) {
	case GP_FILE_TYPE_METADATA:
		return TRUE;
	case GP_FILE_TYPE_AUDIO:
		return (info.audio.fields != 0);
	case GP_FILE_TYPE_PREVIEW:
		return (info.preview.fields != 0);
	case GP_FILE_TYPE_RAW:
	case GP_FILE_TYPE_NORMAL:
		return (info.file.fields != 0);
	default:
		gp_context_error (context, "Unknown file type in camera_file_exists: %d", type);
		return FALSE;
	}
}

int
save_file_to_file (Camera *camera, GPContext *context, Flags flags,
		   const char *folder, const char *filename,
		   CameraFileType type)
{
        int res;
        CameraFile *file;

	if (flags & FLAGS_NEW) {
		CameraFileInfo info;
		
		CR (gp_camera_file_get_info (camera, folder, filename,
					     &info, context));
		switch (type) {
		case GP_FILE_TYPE_PREVIEW:
			if (info.preview.fields & GP_FILE_INFO_STATUS &&
			    info.preview.status == GP_FILE_STATUS_DOWNLOADED)
				return (GP_OK);
			break;
		case GP_FILE_TYPE_NORMAL:
		case GP_FILE_TYPE_RAW:
		case GP_FILE_TYPE_EXIF:
			if (info.file.fields & GP_FILE_INFO_STATUS &&
			    info.file.status == GP_FILE_STATUS_DOWNLOADED)
				return (GP_OK);
			break;
		case GP_FILE_TYPE_AUDIO:
			if (info.audio.fields & GP_FILE_INFO_STATUS &&
			    info.audio.status == GP_FILE_STATUS_DOWNLOADED)
				return (GP_OK);
			break;
		default:
			return (GP_ERROR_NOT_SUPPORTED);
		}
	}
	
        CR (gp_file_new (&file));
        CR (gp_camera_file_get (camera, folder, filename, type,
				file, context));

	if (flags & FLAGS_STDOUT) {
                const char *data;
                long int size;

                CR (gp_file_get_data_and_size (file, &data, &size));

		if (flags & FLAGS_STDOUT_SIZE)
                        printf ("%li\n", size);
                fwrite (data, sizeof(char), size, stdout);
                gp_file_unref (file);
                return (GP_OK);
        }

        res = save_camera_file_to_file (folder, file);

        gp_file_unref (file);

        return (res);
}

/*
  get_file_common() - parse range, download specified files, or their
        thumbnails according to thumbnail argument, and save to files.
*/

static int
get_file_common (const char *arg, CameraFileType type )
{
        gp_log (GP_LOG_DEBUG, "main", "Getting '%s'...", arg);

	p.download_type = type; /* remember for multi download */
	/*
	 * If the user specified the file directly (and not a number),
	 * get that file.
	 */
        if (strchr (arg, '.'))
                return (save_file_to_file (p.camera, p.context, p.flags,
					   p.folder, arg, type));

        switch (type) {
        case GP_FILE_TYPE_PREVIEW:
		CR (for_each_file_in_range (&p, save_thumbnail_action, arg));
		break;
        case GP_FILE_TYPE_NORMAL:
                CR (for_each_file_in_range (&p, save_file_action, arg));
		break;
        case GP_FILE_TYPE_RAW:
                CR (for_each_file_in_range (&p, save_raw_action, arg));
		break;
	case GP_FILE_TYPE_AUDIO:
		CR (for_each_file_in_range (&p, save_audio_action, arg));
		break;
	case GP_FILE_TYPE_EXIF:
		CR (for_each_file_in_range (&p, save_exif_action, arg));
		break;
	case GP_FILE_TYPE_METADATA:
		CR (for_each_file_in_range (&p, save_meta_action, arg));
		break;
        default:
                return (GP_ERROR_NOT_SUPPORTED);
        }

	return (GP_OK);
}


int
capture_generic (CameraCaptureType type, const char *name)
{
	CameraFilePath path, last;
	char *pathsep;
	int result, frames = 0;
	time_t next_pic_time;
	int waittime;

	next_pic_time = time (NULL) + glob_interval;
	if(glob_interval) {
		memset(&last, 0, sizeof(last));
		if (!(p.flags & FLAGS_QUIET))
			printf (_("Time-lapse mode enabled (interval: %ds).\n"),
				glob_interval);
	}

	while(++frames) {
		if (!(p.flags & FLAGS_QUIET) && glob_interval) {
			if(!glob_frames)
				printf (_("Capturing frame #%d...\n"), frames);
			else
				printf (_("Capturing frame #%d/%d...\n"), frames, glob_frames);
		}

		fflush(stdout);

		result =  gp_camera_capture (p.camera, type, &path, p.context);
		if (result != GP_OK) {
			cli_error_print(_("Could not capture."));
			return (result);
		}

		/* If my Canon EOS 10D is set to auto-focus and it is unable to
		 * get focus lock - it will return with *UNKNOWN* as the filename.
		 */
		if (glob_interval && strcmp(path.name, "*UNKNOWN*") == 0) {
			if (!(p.flags & FLAGS_QUIET)) {
				printf (_("Capture failed (auto-focus problem?)...\n"));
				sleep(1);
				continue;
			}
		}

		if (strcmp(path.folder, "/") == 0)
			pathsep = "";
		else
			pathsep = "/";

		if (p.flags & FLAGS_QUIET)
			printf ("%s%s%s\n", path.folder, pathsep, path.name);
		else
			printf (_("New file is in location %s%s%s on the camera\n"),
				path.folder, pathsep, path.name);

		if(!glob_interval) break;

		if(strcmp(path.folder, last.folder)) {
			memcpy(&last, &path, sizeof(last));

			result = set_folder_action(&p, path.folder);
			if (result != GP_OK) {
				cli_error_print(_("Could not set folder."));
				return (result);
			}
		}

		/* XXX: I wonder if there is some way to determine if the camera
		 * is still writing the image because if we don't sleep here,
		 * the image will be corrupt when we attempt to download it.
		 * I assume this is a result of the camera still writing the
		 * data!  I picked 2 seconds as a first try and it seems like
		 * enough time for high-quality JPEG on the Canon EOS 10D (with
		 * a San Disk Ultra II CF).  May need to be increased for RAW
		 * images...  I don't think this is the correct way to do this.
		 */

		/* Marcus Meissner: Fix the camera driver to only return from
		 * camera_capture() when you know it is done ...
		 * Otherwise other drivers suffer.
		 */
#if 0
		sleep(2);
#endif

		result = get_file_common (path.name, GP_FILE_TYPE_NORMAL);
		if (result != GP_OK) {
			cli_error_print (_("Could not get image."));
			if(result == GP_ERROR_FILE_NOT_FOUND) {
				/* Buggy libcanon.so?
				 * Can happen if this was the first capture after a
				 * CF card format, or during a directory roll-over,
				 * ie: CANON100 -> CANON101
				 */
				cli_error_print ( _("Buggy libcanon.so?"));
			}
			return (result);
		}

		if (!(p.flags & FLAGS_QUIET))
			printf (_("Deleting file %s%s%s on the camera\n"),
				path.folder, pathsep, path.name);

		result = delete_file_action (&p, path.name);
		if (result != GP_OK) {
			cli_error_print ( _("Could not delete image."));
			return (result);
		}

		/* Break if we've reached the requested number of frames
		 * to capture.
		 */
		if(glob_frames && frames == glob_frames) break;

#if 0
		/* Marcus Meissner: Before you enable this, try to fix the
		 * camera driver first! camera_exit is NOT necessary for
		 * 2 captures in a row!
		 */

		/* Without this, it seems that the second capture always fails.
		 * That is probably obvious...  for me it was trial n' error.
		 */
		result = gp_camera_exit (p.camera, p.context);
		if (result != GP_OK) {
			cli_error_print (_("Could not close camera connection."));
		}
#endif
		waittime = next_pic_time - time (NULL);
		if (waittime > 0) {
			if (!(p.flags & FLAGS_QUIET) && glob_interval)
				printf (_("Sleeping for %d second(s)...\n"), glob_interval);
			sleep (waittime);
		} else {
			if (!(p.flags & FLAGS_QUIET) && glob_interval)
				printf (_("not sleeping (%d seconds behind schedule)\n"), - waittime);
		}
		next_pic_time += glob_interval;
	}

	return (GP_OK);
}


/* Set/init global variables                                    */
/* ------------------------------------------------------------ */

#ifdef HAVE_PTHREAD

typedef struct _ThreadData ThreadData;
struct _ThreadData {
	Camera *camera;
	unsigned int timeout;
	CameraTimeoutFunc func;
};

static void
thread_cleanup_func (void *data)
{
	ThreadData *td = data;

	free (td);
}

static void *
thread_func (void *data)
{
	ThreadData *td = data;
	time_t t, last;

	pthread_cleanup_push (thread_cleanup_func, td);

	last = time (NULL);
	while (1) {
		t = time (NULL);
		if (t - last > td->timeout) {
			td->func (td->camera, NULL);
			last = t;
		}
		pthread_testcancel ();
	}

	pthread_cleanup_pop (1);
}

static unsigned int
start_timeout_func (Camera *camera, unsigned int timeout,
		    CameraTimeoutFunc func, void *data)
{
	pthread_t tid;
	ThreadData *td;

	td = malloc (sizeof (ThreadData));
	if (!td)
		return 0;
	memset (td, 0, sizeof (ThreadData));
	td->camera = camera;
	td->timeout = timeout;
	td->func = func;

	pthread_create (&tid, NULL, thread_func, td);

	return (tid);
}

static void
stop_timeout_func (Camera *camera, unsigned int id, void *data)
{
	pthread_t tid = id;

	pthread_cancel (tid);
	pthread_join (tid, NULL);
}

#endif

/* Misc functions                                                       */
/* ------------------------------------------------------------------   */

void
cli_error_print (char *format, ...)
{
        va_list         pvar;

        fprintf(stderr, _("ERROR: "));
        va_start(pvar, format);
        vfprintf(stderr, format, pvar);
        va_end(pvar);
        fprintf(stderr, "\n");
}

static void
signal_resize (int signo)
{
	const char *columns;

	columns = getenv ("COLUMNS");
	if (columns && atoi (columns))
		p.cols = atoi (columns);
}

static void
signal_exit (int signo)
{
	/* If we already were told to cancel, abort. */
	if (glob_cancel) {
		if ((p.flags & FLAGS_QUIET) == 0)
			printf (_("\nAborting...\n"));
		if (p.camera)
			gp_camera_unref (p.camera);
		if (p.context)
			gp_context_unref (p.context);
		if ((p.flags & FLAGS_QUIET) == 0)
			printf (_("Aborted.\n"));
		exit (EXIT_FAILURE);
	}

	if ((p.flags & FLAGS_QUIET) == 0)
                printf (_("\nCancelling...\n"));

	glob_cancel = 1;
	glob_interval = 0;
}

/* Main :)                                                              */
/* -------------------------------------------------------------------- */

typedef enum {
	ARG_ABILITIES,
	ARG_ABOUT,
	ARG_AUTO_DETECT,
	ARG_CAPTURE_FRAMES,
	ARG_CAPTURE_INTERVAL,
	ARG_CAPTURE_IMAGE,
	ARG_CAPTURE_MOVIE,
	ARG_CAPTURE_PREVIEW,
	ARG_CAPTURE_SOUND,
	ARG_CONFIG,
	ARG_DEBUG,
	ARG_DELETE_ALL_FILES,
	ARG_DELETE_FILE,
	ARG_FILENAME,
	ARG_FOLDER,
	ARG_FORCE_OVERWRITE,
	ARG_GET_ALL_AUDIO_DATA,
	ARG_GET_ALL_FILES,
	ARG_GET_ALL_METADATA,
	ARG_GET_ALL_RAW_DATA,
	ARG_GET_ALL_THUMBNAILS,
	ARG_GET_AUDIO_DATA,
	ARG_GET_CONFIG,
	ARG_SET_CONFIG,
	ARG_GET_FILE,
	ARG_GET_METADATA,
	ARG_GET_RAW_DATA,
	ARG_GET_THUMBNAIL,
	ARG_LIST_CAMERAS,
	ARG_LIST_CONFIG,
	ARG_LIST_FILES,
	ARG_LIST_FOLDERS,
	ARG_LIST_PORTS,
	ARG_MANUAL,
	ARG_MKDIR,
	ARG_MODEL,
	ARG_NEW,
	ARG_NO_RECURSE,
	ARG_NUM_FILES,
	ARG_PORT,
	ARG_QUIET,
	ARG_RECURSE,
	ARG_RMDIR,
	ARG_SHELL,
	ARG_SHOW_EXIF,
	ARG_SHOW_INFO,
	ARG_SPEED,
	ARG_STDOUT,
	ARG_STDOUT_SIZE,
	ARG_SUMMARY,
	ARG_UPLOAD_FILE,
	ARG_UPLOAD_METADATA,
	ARG_USBID,
	ARG_VERSION,
	ARG_WAIT_EVENT,
} Arg;

typedef enum {
	CALLBACK_PARAMS_TYPE_PREINITIALIZE,
	CALLBACK_PARAMS_TYPE_INITIALIZE,
	CALLBACK_PARAMS_TYPE_QUERY,
	CALLBACK_PARAMS_TYPE_RUN
} CallbackParamsType;

typedef struct _CallbackParams CallbackParams;
struct _CallbackParams {
	CallbackParamsType type;
	union {
		/*
		 * CALLBACK_PARAMS_TYPE_RUN,
		 * CALLBACK_PARAMS_TYPE_INITIALIZE,
		 * CALLBACK_PARAMS_TYPE_PREINITIALIZE,
		 */
		int r;

		/* CALLBACK_PARAMS_TYPE_QUERY */
		struct {
			Arg arg;
			char found;
		} q;
	} p;
};

static void
cb_arg (poptContext ctx, enum poptCallbackReason reason,
	const struct poptOption *opt, const char *arg, void *data)
{
	CallbackParams *params = (CallbackParams *) data;
	int usb_product, usb_vendor, usb_product_modified, usb_vendor_modified;

	/* Check if we are only to query. */
	if (params->type == CALLBACK_PARAMS_TYPE_QUERY) {
		if (opt->val == params->p.q.arg)
			params->p.q.found = 1;
		return;
	}

	/* Check if we are only to pre-initialize. */
	if (params->type == CALLBACK_PARAMS_TYPE_PREINITIALIZE) {
		switch (opt->val) {
		case ARG_USBID:
			gp_log (GP_LOG_DEBUG, "main", "Overriding USB "
				"IDs to '%s'...", arg);
			if (sscanf (arg, "0x%x:0x%x=0x%x:0x%x",
				    &usb_vendor_modified,
				    &usb_product_modified, &usb_vendor,
				    &usb_product) != 4) {
				printf (_("Use the following syntax a:b=c:d "
					  "to treat any USB device detected "
					  "as a:b as c:d instead. "
					  "a b c d should be hexadecimal "
					  "numbers beginning with '0x'.\n"));
				params->p.r = GP_ERROR_BAD_PARAMETERS;
				break;
			}
			params->p.r = override_usbids_action (&p, usb_vendor,
					usb_product, usb_vendor_modified,
					usb_product_modified);
			break;
		default:
			break;
		}
		return;
	}

	/* Check if we are only to initialize. */
	if (params->type == CALLBACK_PARAMS_TYPE_INITIALIZE) {
		switch (opt->val) {
		case ARG_FILENAME:
			params->p.r = set_filename_action (&p, arg);
			break;
		case ARG_FOLDER:
			params->p.r = set_folder_action (&p, arg);
			break;
		case ARG_FORCE_OVERWRITE:
			p.flags |= FLAGS_FORCE_OVERWRITE;
			break;
		case ARG_MODEL:
			gp_log (GP_LOG_DEBUG, "main", "Processing 'model' "
				"option ('%s')...", arg);
			params->p.r = action_camera_set_model (&p, arg);
			break;
		case ARG_NEW:
			p.flags |= FLAGS_NEW;
			break;
		case ARG_NO_RECURSE:
			p.flags &= ~FLAGS_RECURSE;
			break;
		case ARG_PORT:
			gp_log (GP_LOG_DEBUG, "main", "Processing 'port' "
				"option ('%s')...", arg);
			params->p.r = action_camera_set_port (&p, arg);
			break;
		case ARG_QUIET:
			p.flags |= FLAGS_QUIET;
			break;
		case ARG_RECURSE:
			p.flags |= FLAGS_RECURSE;
			break;
		case ARG_SPEED:
			params->p.r = action_camera_set_speed (&p, atoi (arg));
			break;
		case ARG_STDOUT:
			p.flags |= FLAGS_QUIET | FLAGS_STDOUT;
			break;
		case ARG_CAPTURE_FRAMES:
			glob_frames = atoi(arg);
			break;
		case ARG_CAPTURE_INTERVAL:
			glob_interval = atoi(arg);
			break;
		case ARG_STDOUT_SIZE:
			p.flags |= FLAGS_QUIET | FLAGS_STDOUT | FLAGS_STDOUT_SIZE;
			break;
		case ARG_VERSION:
			params->p.r = print_version_action (&p);
			break;
		default:
			break;
		}
		return;
	}

	switch (opt->val) {
	case ARG_ABILITIES:
		params->p.r = action_camera_show_abilities (&p);
		break;
	case ARG_ABOUT:
		params->p.r = action_camera_about (&p);
		break;
	case ARG_AUTO_DETECT:
		params->p.r = auto_detect_action (&p);
		break;
	case ARG_CAPTURE_IMAGE:
		params->p.r = capture_generic (GP_CAPTURE_IMAGE, arg);
		break;
	case ARG_CAPTURE_MOVIE:
		params->p.r = capture_generic (GP_CAPTURE_MOVIE, arg);
		break;
	case ARG_CAPTURE_PREVIEW:
		params->p.r = action_camera_capture_preview (&p);
		break;
	case ARG_CAPTURE_SOUND:
		params->p.r = capture_generic (GP_CAPTURE_SOUND, arg);
		break;
	case ARG_CONFIG:
#ifdef HAVE_CDK
		params->p.r = gp_cmd_config (p.camera, p.context);
#else
		gp_context_error (p.context,
				  _("gphoto2 has been compiled without "
				    "support for CDK."));
		params->p.r = GP_ERROR_NOT_SUPPORTED;
#endif
		break;
	case ARG_DELETE_ALL_FILES:
		params->p.r = for_each_folder (&p, delete_all_action);
		break;
	case ARG_DELETE_FILE:
		p.multi_type = MULTI_DELETE;
		/* Did the user specify a file or a range? */
		if (strchr (arg, '.')) {
			params->p.r = delete_file_action (&p, arg);
			break;
		}
		params->p.r = for_each_file_in_range (&p,
					delete_file_action, arg);
		break;
	case ARG_GET_ALL_AUDIO_DATA:
		params->p.r = for_each_file (&p, save_all_audio_action);
		break;
	case ARG_GET_ALL_FILES:
		params->p.r = for_each_file (&p, save_file_action);
		break;
	case ARG_GET_ALL_METADATA:
		params->p.r = for_each_file (&p, save_meta_action);
		break;
	case ARG_GET_ALL_RAW_DATA:
		params->p.r = for_each_file (&p, save_raw_action);
		break;
	case ARG_GET_ALL_THUMBNAILS:
		params->p.r = for_each_file (&p, save_thumbnail_action);
		break;
	case ARG_GET_AUDIO_DATA:
		p.multi_type = MULTI_DOWNLOAD;
		params->p.r = get_file_common (arg, GP_FILE_TYPE_AUDIO);
		break;
	case ARG_GET_METADATA:
		p.multi_type = MULTI_DOWNLOAD;
		params->p.r = get_file_common (arg, GP_FILE_TYPE_METADATA);
		break;
	case ARG_GET_FILE:
		p.multi_type = MULTI_DOWNLOAD;
		params->p.r = get_file_common (arg, GP_FILE_TYPE_NORMAL);
		break;
	case ARG_GET_THUMBNAIL:
		p.multi_type = MULTI_DOWNLOAD;
		params->p.r = get_file_common (arg, GP_FILE_TYPE_PREVIEW);
		break;
	case ARG_GET_RAW_DATA:
		p.multi_type = MULTI_DOWNLOAD;
		params->p.r = get_file_common (arg, GP_FILE_TYPE_RAW);
		break;
	case ARG_LIST_CAMERAS:
		params->p.r = list_cameras_action (&p);
		break;
	case ARG_LIST_FILES:
		params->p.r = for_each_folder (&p, list_files_action);
		break;
	case ARG_LIST_FOLDERS:
		params->p.r = for_each_folder (&p, list_folders_action);
		break;
	case ARG_LIST_PORTS:
		params->p.r = list_ports_action (&p);
		break;
	case ARG_MANUAL:
		params->p.r = action_camera_manual (&p);
		break;
	case ARG_RMDIR:
		params->p.r = gp_camera_folder_remove_dir (p.camera,
				p.folder, arg, p.context);
		break;
	case ARG_NUM_FILES:
		params->p.r = num_files_action (&p);
		break;
	case ARG_MKDIR:
		params->p.r = gp_camera_folder_make_dir (p.camera,
				p.folder, arg, p.context);
		break;
	case ARG_SHELL:
		params->p.r = shell_prompt (&p);
		break;
	case ARG_SHOW_EXIF:
		/* Did the user specify a file or a range? */
		if (strchr (arg, '.')) { 
			params->p.r = print_exif_action (&p, arg); 
			break; 
		} 
		params->p.r = for_each_file_in_range (&p, 
						 print_exif_action, arg); 
		break;
	case ARG_SHOW_INFO:

		/* Did the user specify a file or a range? */
		if (strchr (arg, '.')) {
			params->p.r = print_info_action (&p, arg);
			break;
		}
		params->p.r = for_each_file_in_range (&p,
						 print_info_action, arg);
		break;
	case ARG_SUMMARY:
		params->p.r = action_camera_summary (&p);
		break;
	case ARG_UPLOAD_FILE:
		p.multi_type = MULTI_UPLOAD;
		params->p.r = action_camera_upload_file (&p, p.folder, arg);
		break;
	case ARG_UPLOAD_METADATA:
		p.multi_type = MULTI_UPLOAD_META;
		params->p.r = action_camera_upload_metadata (&p, p.folder, arg);
		break;
	case ARG_LIST_CONFIG:
		params->p.r = list_config_action (&p);
		break;
	case ARG_GET_CONFIG:
		params->p.r = get_config_action (&p, arg);
		break;
	case ARG_SET_CONFIG: {
		char *name, *value;

		if (strchr (arg, '=') == NULL) {
			params->p.r = GP_ERROR_BAD_PARAMETERS;
			break;
		}
		name  = strdup (arg);
		value = strchr (name, '=');
		*value = '\0';
		value++;
		params->p.r = set_config_action (&p, name, value);
		free (name);
		break;
	}
	case ARG_WAIT_EVENT:
		params->p.r = action_camera_wait_event (&p);
		break;
	default:
		break;
	};
};


static void
report_failure (int result, int argc, char **argv)
{
	if (result >= 0)
		return;

	if (result == GP_ERROR_CANCEL) {
		fprintf (stderr, _("Operation cancelled.\n"));
		return;
	}

	fprintf (stderr, _("*** Error (%i: '%s') ***       \n\n"), result,
		gp_result_as_string (result));
	if (glob_debug == -1) {
		int n;
		printf (_("For debugging messages, please use "
			  "the --debug option.\n"
			  "Debugging messages may help finding a "
			  "solution to your problem.\n"
			  "If you intend to send any error or "
			  "debug messages to the gphoto\n"
			  "developer mailing list "
			  "<gphoto-devel@lists.sourceforge.net>, please run\n"
			  "gphoto2 as follows:\n\n"));

		/*
		 * Print the exact command line to assist
		 * l^Husers
		 */
		printf ("    env LANG=C gphoto2 --debug");
		for (n = 1; n < argc; n++) {
			if (argv[n][0] == '-')
				printf(" %s",argv[n]);
			else
				printf(" \"%s\"",argv[n]);
		}
		printf ("\n\n");
		printf ("Please make sure there is sufficient quoting "
			"around the arguments.\n\n");
	}
}

#define CR_MAIN(result)					\
{							\
	int r = (result);				\
							\
	if (r < 0) {					\
		report_failure (r, argc, argv);		\
		gp_params_exit (&p);			\
		return (EXIT_FAILURE);			\
	}						\
}

int
main (int argc, char **argv)
{
	CallbackParams params;
	poptContext ctx;
	const struct poptOption options[] = {
		POPT_AUTOHELP
		{NULL, '\0', POPT_ARG_CALLBACK,
		 (void *) &cb_arg, 0, (char *) &params, NULL},
		{"debug", '\0', POPT_ARG_NONE, NULL, ARG_DEBUG,
		 N_("Turn on debugging"), NULL},
		{"quiet", '\0', POPT_ARG_NONE, NULL, ARG_QUIET,
		 N_("Quiet output (default=verbose)"), NULL},
		{"force-overwrite", '\0', POPT_ARG_NONE, NULL,
		 ARG_FORCE_OVERWRITE, N_("Overwrite files without asking")},
		{"version", 'v', POPT_ARG_NONE, NULL, ARG_VERSION,
		 N_("Display version and exit"), NULL},
		{"list-cameras", '\0', POPT_ARG_NONE, NULL, ARG_LIST_CAMERAS,
		 N_("List supported camera models"), NULL},
		{"list-ports", '\0', POPT_ARG_NONE, NULL, ARG_LIST_PORTS,
		 N_("List supported port devices"), NULL},
		{"stdout", '\0', POPT_ARG_NONE, NULL, ARG_STDOUT,
		 N_("Send file to stdout"), NULL},
		{"stdout-size", '\0', POPT_ARG_NONE, NULL, ARG_STDOUT_SIZE,
		 N_("Print filesize before data"), NULL},
		{"auto-detect", '\0', POPT_ARG_NONE, NULL, ARG_AUTO_DETECT,
		 N_("List auto-detected cameras"), NULL},
		{"port", '\0', POPT_ARG_STRING, NULL, ARG_PORT,
		 N_("Specify port device"), N_("path")},
		{"speed", '\0', POPT_ARG_INT, NULL, ARG_SPEED,
		 N_("Specify serial transfer speed"), N_("speed")},
		{"camera", '\0', POPT_ARG_STRING, NULL, ARG_MODEL,
		 N_("Specify camera model"), N_("model")},
		{"filename", '\0', POPT_ARG_STRING, NULL, ARG_FILENAME,
		 N_("Specify a filename"), N_("filename")},
		{"usbid", '\0', POPT_ARG_STRING, NULL, ARG_USBID,
		 N_("(expert only) Override USB IDs"), N_("usbid")},
		{"abilities", 'a', POPT_ARG_NONE, NULL, ARG_ABILITIES,
		 N_("Display camera abilities"), NULL},
		{"folder", 'f', POPT_ARG_STRING, NULL, ARG_FOLDER,
		 N_("Specify camera folder (default=\"/\")"), N_("folder")},
		{"recurse", 'R', POPT_ARG_NONE, NULL, ARG_RECURSE,
		 N_("Recursion (default for download)"), NULL},
		{"no-recurse", '\0', POPT_ARG_NONE, NULL, ARG_NO_RECURSE,
		 N_("No recursion (default for deletion)"), NULL},
		{"new", '\0', POPT_ARG_NONE, NULL, ARG_NEW,
		 N_("Process new files only"), NULL},
		{"list-folders", 'l', POPT_ARG_NONE, NULL, ARG_LIST_FOLDERS,
		 N_("List folders in folder"), NULL},
		{"list-files", 'L', POPT_ARG_NONE, NULL, ARG_LIST_FILES,
		 N_("List files in folder"), NULL},
		{"mkdir", 'm', POPT_ARG_STRING, NULL, ARG_MKDIR,
		 N_("Create a directory"), NULL},
		{"rmdir", 'r', POPT_ARG_STRING, NULL, ARG_RMDIR,
		 N_("Remove a directory"), NULL},
		{"num-files", 'n', POPT_ARG_NONE, NULL, ARG_NUM_FILES,
		 N_("Display number of files"), NULL},
		{"get-file", 'p', POPT_ARG_STRING, NULL, ARG_GET_FILE,
		 N_("Get files given in range"), NULL},
		{"get-all-files", 'P', POPT_ARG_NONE, NULL, ARG_GET_ALL_FILES,
		 N_("Get all files from folder"), NULL},
		{"get-thumbnail", 't', POPT_ARG_STRING, NULL, ARG_GET_THUMBNAIL,
		 N_("Get thumbnails given in range"), NULL},
		{"get-all-thumbnails", 'T', POPT_ARG_NONE, 0,
		 ARG_GET_ALL_THUMBNAILS,
		 N_("Get all thumbnails from folder"), NULL},
		{"get-metadata", '\0', POPT_ARG_STRING, NULL, ARG_GET_METADATA,
		 N_("Get metadata given in range"), NULL},
		{"get-all-metadata", '\0', POPT_ARG_STRING, NULL, ARG_GET_ALL_METADATA,
		 N_("Get all metadata from folder"), NULL},
		{"upload-metadata", '\0', POPT_ARG_STRING, NULL, ARG_UPLOAD_METADATA,
		 N_("Upload metadata for file"), NULL},
		{"get-raw-data", '\0', POPT_ARG_STRING, NULL,
		 ARG_GET_RAW_DATA,
		 N_("Get raw data given in range"), NULL},
		{"get-all-raw-data", '\0', POPT_ARG_NONE, NULL,
		 ARG_GET_ALL_RAW_DATA,
		 N_("Get all raw data from folder"), NULL},
		{"get-audio-data", '\0', POPT_ARG_STRING, NULL,
		 ARG_GET_AUDIO_DATA,
		 N_("Get audio data given in range"), NULL},
		{"get-all-audio-data", '\0', POPT_ARG_NONE, NULL,
		 ARG_GET_ALL_AUDIO_DATA,
		 N_("Get all audio data from folder"), NULL},
		{"delete-file", 'd', POPT_ARG_STRING, NULL, ARG_DELETE_FILE,
		 N_("Delete files given in range"), NULL},
		{"delete-all-files", 'D', POPT_ARG_NONE, NULL,
		 ARG_DELETE_ALL_FILES, N_("Delete all files in folder"), NULL},
		{"upload-file", 'u', POPT_ARG_STRING, NULL, ARG_UPLOAD_FILE,
		 N_("Upload a file to camera"), NULL},
#ifdef HAVE_CDK
		{"config", '\0', POPT_ARG_NONE, NULL, ARG_CONFIG,
		 N_("Configure"), NULL},
#endif
		{"list-config", '\0', POPT_ARG_NONE, NULL, ARG_LIST_CONFIG,
		 N_("List configuration tree"), NULL},
		{"get-config", '\0', POPT_ARG_STRING, NULL, ARG_GET_CONFIG,
		 N_("Get configuration value"), NULL},
		{"set-config", '\0', POPT_ARG_STRING, NULL, ARG_SET_CONFIG,
		 N_("Set configuration value"), NULL},
		{"wait-event", '\0', POPT_ARG_NONE, NULL, ARG_WAIT_EVENT,
		 N_("Wait for event from camera"), NULL},
		{"capture-preview", '\0', POPT_ARG_NONE, NULL,
		 ARG_CAPTURE_PREVIEW,
		 N_("Capture a quick preview"), NULL},
		{"frames", 'F', POPT_ARG_INT, NULL, ARG_CAPTURE_FRAMES,
		 N_("Set number of frames to capture (default=infinite)"), N_("count")},
		{"interval", 'I', POPT_ARG_INT, NULL, ARG_CAPTURE_INTERVAL,
		 N_("Set capture interval in seconds"), N_("seconds")},
		{"capture-image", '\0', POPT_ARG_NONE, NULL,
		 ARG_CAPTURE_IMAGE, N_("Capture an image"), NULL},
		{"capture-movie", '\0', POPT_ARG_NONE, NULL,
		 ARG_CAPTURE_MOVIE, N_("Capture a movie"), NULL},
		{"capture-sound", '\0', POPT_ARG_NONE, NULL,
		 ARG_CAPTURE_SOUND, N_("Capture an audio clip"), NULL},
#ifdef HAVE_LIBEXIF
		{"show-exif", '\0', POPT_ARG_STRING, NULL, ARG_SHOW_EXIF,
		 N_("Show EXIF information"), NULL},
#endif
		{"show-info", '\0', POPT_ARG_STRING, NULL, ARG_SHOW_INFO,
		 N_("Show info"), NULL},
		{"summary", '\0', POPT_ARG_NONE, NULL, ARG_SUMMARY,
		 N_("Show summary"), NULL},
		{"manual", '\0', POPT_ARG_NONE, NULL, ARG_MANUAL,
		 N_("Show camera driver manual"), NULL},
		{"about", '\0', POPT_ARG_NONE, NULL, ARG_ABOUT,
		 N_("About the camera driver manual"), NULL},
		{"shell", '\0', POPT_ARG_NONE, NULL, ARG_SHELL,
		 N_("gPhoto shell"), NULL},
		POPT_TABLEEND
	};
	CameraAbilities a;
	GPPortInfo info;
	int result = GP_OK;

	/* For translation */
	setlocale (LC_ALL, "");
        bindtextdomain (PACKAGE, LOCALEDIR);
        textdomain (PACKAGE);

	/* Create the global variables. */
	gp_params_init (&p);

	/* Figure out the width of the terminal and watch out for changes */
	signal_resize (0);
	signal (SIGWINCH, signal_resize);

	/* Prepare processing options. */
	ctx = poptGetContext (PACKAGE, argc, (const char **) argv, options, 0);
	if (argc <= 1) {
		poptPrintUsage (ctx, stdout, 0);
		return (0);
	}

	/*
	 * Do we need debugging output? While we are at it, scan the 
	 * options for bad ones.
	 */
	params.type = CALLBACK_PARAMS_TYPE_QUERY;
	params.p.q.found = 0;
	params.p.q.arg = ARG_DEBUG;
	poptResetContext (ctx);
	while ((result = poptGetNextOpt (ctx)) >= 0);
	if (result == POPT_ERROR_BADOPT) {
		poptPrintUsage (ctx, stderr, 0);
		return (EXIT_FAILURE);
	}
	if (params.p.q.found) {
		CR_MAIN (debug_action (&p));
	}

	/* Initialize. */
#ifdef HAVE_PTHREAD
	gp_camera_set_timeout_funcs (p.camera, start_timeout_func,
				     stop_timeout_func, NULL);
#endif
	params.type = CALLBACK_PARAMS_TYPE_PREINITIALIZE;
	params.p.r = GP_OK;
	poptResetContext (ctx);
	while ((params.p.r >= 0) && (poptGetNextOpt (ctx) >= 0));
	params.type = CALLBACK_PARAMS_TYPE_INITIALIZE;
	poptResetContext (ctx);
	while ((params.p.r >= 0) && (poptGetNextOpt (ctx) >= 0));
	CR_MAIN (params.p.r);

#define CHECK_OPT(o)					\
	if (!params.p.q.found) {			\
		params.p.q.arg = (o);			\
		poptResetContext (ctx);			\
		while (poptGetNextOpt (ctx) >= 0);	\
	}

	/* If we need a camera, make sure we've got one. */
	CR_MAIN (gp_camera_get_abilities (p.camera, &a));
	CR_MAIN (gp_camera_get_port_info (p.camera, &info));

	params.type = CALLBACK_PARAMS_TYPE_QUERY;
	params.p.q.found = 0;
	CHECK_OPT (ARG_ABILITIES);
	CHECK_OPT (ARG_CAPTURE_IMAGE);
	CHECK_OPT (ARG_CAPTURE_MOVIE);
	CHECK_OPT (ARG_CAPTURE_PREVIEW);
	CHECK_OPT (ARG_CAPTURE_SOUND);
	CHECK_OPT (ARG_CONFIG);
	CHECK_OPT (ARG_DELETE_ALL_FILES);
	CHECK_OPT (ARG_DELETE_FILE);
	CHECK_OPT (ARG_GET_ALL_AUDIO_DATA);
	CHECK_OPT (ARG_GET_ALL_FILES);
	CHECK_OPT (ARG_GET_ALL_RAW_DATA);
	CHECK_OPT (ARG_GET_ALL_THUMBNAILS);
	CHECK_OPT (ARG_GET_AUDIO_DATA);
	CHECK_OPT (ARG_GET_FILE);
	CHECK_OPT (ARG_GET_RAW_DATA);
	CHECK_OPT (ARG_GET_THUMBNAIL);
	CHECK_OPT (ARG_LIST_FILES);
	CHECK_OPT (ARG_LIST_FOLDERS);
	CHECK_OPT (ARG_MANUAL);
	CHECK_OPT (ARG_MKDIR);
	CHECK_OPT (ARG_NUM_FILES);
	CHECK_OPT (ARG_RMDIR);
	CHECK_OPT (ARG_SHELL);
	CHECK_OPT (ARG_SHOW_EXIF);
	CHECK_OPT (ARG_SHOW_INFO);
	CHECK_OPT (ARG_SUMMARY);
	CHECK_OPT (ARG_UPLOAD_FILE);
	if (params.p.q.found &&
	    (!strcmp (a.model, "") || (info.type == GP_PORT_NONE))) {
		int count;
		const char *model = NULL, *path = NULL;
		CameraList *list;
		char buf[1024];
		int use_auto = 1;

		gp_log (GP_LOG_DEBUG, "main", "The user has not specified "
			"both a model and a port. Try to figure them out.");

		_get_portinfo_list(&p);
		CR_MAIN (gp_list_new (&list)); /* no freeing below */
		CR_MAIN (gp_abilities_list_detect (p.abilities_list, p.portinfo_list,
						   list, p.context));
		CR_MAIN (count = gp_list_count (list));
                if (count == 1) {
                        /* Exactly one camera detected */
			CR_MAIN (gp_list_get_name (list, 0, &model));
			CR_MAIN (gp_list_get_value (list, 0, &path));
			if (a.model[0] && strcmp(a.model,model)) {
				CameraAbilities alt;
				int m;

				CR_MAIN (m = gp_abilities_list_lookup_model (p.abilities_list, model));
				CR_MAIN (gp_abilities_list_get_abilities (p.abilities_list, m, &alt));

				if ((a.port == GP_PORT_USB) && (alt.port == GP_PORT_USB)) {
					if (	(a.usb_vendor  == alt.usb_vendor)  &&
						(a.usb_product == alt.usb_product)
					)
						use_auto = 0;
				}
			}

			if (use_auto) {
				CR_MAIN (action_camera_set_model (&p, model));
			}
			CR_MAIN (action_camera_set_port (&p, path));

                } else if (!count) {
			/*
			 * No camera detected. Have a look at the settings.
			 * Ignore errors here.
			 */
                        if (gp_setting_get ("gphoto2", "model", buf) >= 0)
				action_camera_set_model (&p, buf);
			if (gp_setting_get ("gphoto2", "port", buf) >= 0)
				action_camera_set_port (&p, buf);

		} else {
			/* If --camera override, search the model with the same USB ID. */
			if (a.model[0]) {
				int i;
				const char *xmodel;

				for (i=0;i<count;i++) {
					CameraAbilities alt;
					int m;

					gp_list_get_name (list, i, &xmodel);
					CR_MAIN (m = gp_abilities_list_lookup_model (p.abilities_list, xmodel));
					CR_MAIN (gp_abilities_list_get_abilities (p.abilities_list, m, &alt));

					if ((a.port == GP_PORT_USB) && (alt.port == GP_PORT_USB)) {
						if (	(a.usb_vendor  == alt.usb_vendor)  &&
							(a.usb_product == alt.usb_product)
						) {
							use_auto = 0;
							CR_MAIN (gp_list_get_value (list, i, &path));
							CR_MAIN (action_camera_set_port (&p, path));
							break;
						}
					}
				}
			}
			if (use_auto) {
				/* More than one camera detected */
				/*FIXME: Let the user choose from the list!*/
				CR_MAIN (gp_list_get_name (list, 0, &model));
				CR_MAIN (gp_list_get_value (list, 0, &path));
				CR_MAIN (action_camera_set_model (&p, model));
				CR_MAIN (action_camera_set_port (&p, path));
			}
                }
		gp_list_free (list);
        }

	/*
	 * Recursion is too dangerous for deletion. Only turn it on if
	 * explicitely specified.
	 */
	params.type = CALLBACK_PARAMS_TYPE_QUERY;
	params.p.q.found = 0;
	params.p.q.arg = ARG_DELETE_FILE;
	poptResetContext (ctx);
	while (poptGetNextOpt (ctx) >= 0);
	if (!params.p.q.found) {
		params.p.q.arg = ARG_DELETE_ALL_FILES;
		poptResetContext (ctx);
		while (poptGetNextOpt (ctx) >= 0);
	}
	if (params.p.q.found) {
		params.p.q.found = 0;
		params.p.q.arg = ARG_RECURSE;
		poptResetContext (ctx);
		while (poptGetNextOpt (ctx) >= 0);
		if (!params.p.q.found)
			p.flags &= ~FLAGS_RECURSE;
	}

        signal (SIGINT, signal_exit);

	/* If we are told to be quiet, do so. */
	params.type = CALLBACK_PARAMS_TYPE_QUERY;
	params.p.q.found = 0;
	params.p.q.arg = ARG_QUIET;
	poptResetContext (ctx);
	while (poptGetNextOpt (ctx) >= 0);
	if (params.p.q.found)
		p.flags |= FLAGS_QUIET;

	/* Go! */
	params.type = CALLBACK_PARAMS_TYPE_RUN;
	poptResetContext (ctx);
	params.p.r = GP_OK;
	while ((params.p.r >= GP_OK) && (poptGetNextOpt (ctx) >= 0));

	switch (p.multi_type) {
	case MULTI_UPLOAD: {
		const char *arg;

		while ((params.p.r >= GP_OK) && (NULL != (arg = poptGetArg (ctx)))) {
			CR_MAIN (action_camera_upload_file (&p, p.folder, arg));
		}
		break;
	}
	case MULTI_UPLOAD_META: {
		const char *arg;

		while ((params.p.r >= GP_OK) && (NULL != (arg = poptGetArg (ctx)))) {
			CR_MAIN (action_camera_upload_metadata (&p, p.folder, arg));
		}
		break;
	}
	case MULTI_DELETE: {
		const char *arg;

		while ((params.p.r >= GP_OK) && (NULL != (arg = poptGetArg (ctx)))) {
			CR_MAIN (delete_file_action (&p, arg));
		}
		break;
	}
	case MULTI_DOWNLOAD: {
		const char *arg;

		while ((params.p.r >= GP_OK) && (NULL != (arg = poptGetArg (ctx)))) {
			CR_MAIN (get_file_common (arg, p.download_type ));
		}
		break;
	}
	default:
		break;
	}

	CR_MAIN (params.p.r);

	/* FIXME: These two OS/2 env var checks should happen before
	 *        we load the camlibs */

#ifdef OS2 /*check if environment is set otherwise quit*/
        if(CAMLIBS==NULL)
        {
printf(_("gPhoto2 for OS/2 requires you to set the enviroment value CAMLIBS \
to the location of the camera libraries. \
e.g. SET CAMLIBS=C:\\GPHOTO2\\CAM\n"));
                exit(EXIT_FAILURE);
        }
#endif

#ifdef OS2 /*check if environment is set otherwise quit*/
        if(IOLIBS==NULL)
        {
printf(_("gPhoto2 for OS/2 requires you to set the enviroment value IOLIBS \
to the location of the io libraries. \
e.g. SET IOLIBS=C:\\GPHOTO2\\IOLIB\n"));
                exit(EXIT_FAILURE);
        }
#endif

	gp_params_exit (&p);

        return (EXIT_SUCCESS);
}
