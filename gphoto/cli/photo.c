/* $Id$

 * Copyright (C) 1999  Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Log$
 * Revision 1.1  1999/09/03 18:21:08  ole
 * This is my skeleton for photo, an imlib/X independent
 * command line interface that gradually will be able to
 * call gPhoto libraries, use libjpeg for file i/o, and
 * be able to run in console/screen mode without X.
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*  #include "config.h" */
/*  #include "main.h" */
/*  #include "gphoto.h" */
/*  #include "photo.h" */

/* FIXME: Hardcoded PACKAGE and VERSION, remove this for automake. (Ole) */

#define PACKAGE "photo"
#define VERSION "0.0.1"

/* For autoconf.  (Ole) */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* FIXME: add getopt check to ../configure.in  (Ole) */
#ifndef HAVE_GETOPT_LONG
#include "getopt.h"
#else
#include <getopt.h>
#endif

void 
test_file_name (char *fname)
{
  FILE *fp;

  if (fname == NULL)
    {
      fprintf (stderr, "The first option you must specify in the command line is \"--file\".\n");
      exit (1);
    }
  else if ((fp = fopen (fname, "r")))
    {
      fprintf (stderr, "%s already exists!\n", fname);
      fclose (fp);
      exit (2);

    }
  else if (fname[0] == '-')
    {
      fprintf (stderr, "Did you specify a file name?  %s looks more like an option to me.\n", fname);
    }
  else if (fname == NULL)
    {
      fprintf (stderr, "You have to specify a file name with the \"--file\" option.\n");
      exit (3);
    }
}

void 
cli_print_version (void)
{
  printf ("GNU %s %s\n", PACKAGE, VERSION);
}

void 
cli_save_image (int image_number, char *fname)
{
  test_file_name (fname);
  printf ("FIX: Saving image number %i as %s\n", image_number, fname);
}

void 
cli_save_thumbnail (int thumbnail_number, char *fname)
{
  test_file_name (fname);
  printf ("FIX Saving thumbnail %i as %s\n", thumbnail_number, fname);
}

void 
cli_camera_summary ()
{
  printf ("FIXME: Printing camera summary...\n");
}

void 
cli_delete_image (int image_number)
{
  printf ("FIXME: Deleting image %i\n", image_number);
}

void 
cli_display_numbers ()
{
  printf ("FIXME: Displaying number of images in camera.\n");
}

void 
cli_save_live_preview (char *fname)
{
  test_file_name (fname);
  printf ("FIXME: Saving live preview as %s\n", fname);
}

void 
cli_print_help (void)
{
  printf ("GNU photo v.%s - command line tool for gPhoto libraries.\n", VERSION);
  printf ("Copyright (C) 1999 Free Software Foundation, Inc.\n\n");
  printf ("Usage: %s [-h|--help] [-V|--version] [-fSTRING|--filename=STRING] [-sINT|--save-image=INT] [-tINT|--save-thumbnail=INT] [-dINT|--delete-image=INT] [-c|--camera-summary] [-n|--number-of-pictures] [-l|--save-live-preview] \n\
   -h         --help                 Print help and exit\n\
   -V         --version              Print version and exit\n\
   -fSTRING   --filename=STRING      Specify save pattern\n\
   -sINT      --save-image=INT       Save image number\n\
   -tINT      --save-thumbnail=INT   Save thumbnail number\n\
   -dINT      --delete-image=INT     Delete image number\n\
   -c         --camera-summary       Display camera summary\n\
   -n         --number-of-pictures   Display number of pictures\n\
   -l         --save-live-preview    Save live preview\n\
", PACKAGE);
  printf ("\nGNU photo is free software; you can redistribute it and/or\n");
  printf ("modify it under the terms of the GNU General Public License \n");
  printf ("as published by the Free Software Foundation; either version \n");
  printf ("2 of the License, or any later version.  Visit gphoto/COPYING\n");
  printf ("in the source distribution to read the GNU GPL license terms.\n\n");
  printf ("Report bugs and camera reports to gphoto-devel@gphoto.org.\n");
  printf ("See http://www.gphoto.org for the latest news and updates.\n");

}

char *
gengetopt_strdup (char *s)
{
  char *n, *pn, *ps = s;
  while (*ps)
    ps++;
  n = malloc (1 + ps - s);
  if (n != NULL)
    {
      for (ps = s, pn = n; *ps; ps++, pn++)
	*pn = *ps;
      *pn = 0;
    }
  return n;
}

int 
main (int argc, char **argv)
{
  int c;			/* Character of the parsed option.  */

  char *filename_arg = NULL;	/* Specify save pattern.  */
  int save_image_arg;		/* Save image number.  */
  int save_thumbnail_arg;	/* Save thumbnail number.  */
  int delete_image_arg;		/* Delete image number.  */
  int filename_given = 0;	/* Wheter filename was given.  */
  int save_image_given = 0;	/* Wheter save-image was given.  */
  int save_thumbnail_given = 0;	/* Wheter save-thumbnail was given.  */
  int delete_image_given = 0;	/* Wheter delete-image was given.  */

#define clear_args()
  {
    if (filename_arg != NULL)
      free (filename_arg);
  }

  while (1)
    {
      int option_index = 0;
      static struct option long_options[] =
      {
	{"help", 0, NULL, 'h'},
	{"version", 0, NULL, 'V'},
	{"filename", 1, NULL, 'f'},
	{"save-image", 1, NULL, 's'},
	{"save-thumbnail", 1, NULL, 't'},
	{"delete-image", 1, NULL, 'd'},
	{"camera-summary", 0, NULL, 'c'},
	{"number-of-pictures", 0, NULL, 'n'},
	{"save-live-preview", 0, NULL, 'l'},
	{NULL, 0, NULL, 0}
      };

      c = getopt_long (argc, argv, "hVf:s:t:d:cnl",
		       long_options, &option_index);

      if (c == -1)
	break;			/* Exit from `while (1)' loop.  */

      switch (c)
	{
	case 'h':		/* Print help and exit.  */
	  clear_args ();
	  cli_print_help ();
	  exit (0);

	case 'V':		/* Print version and exit.  */
	  clear_args ();
	  cli_print_version ();
	  exit (0);

	case 'f':		/* Specify save pattern.  */
	  if (filename_given)
	    {
	      fprintf (stderr, "%s: `--filename' (`-f') option redeclared\n", PACKAGE);
	      clear_args ();
	      cli_print_help ();
	      exit (1);
	    }
	  filename_given = 1;
	  filename_arg = gengetopt_strdup (optarg);
	  test_file_name (filename_arg);
	  printf ("Filename input was %s\n", filename_arg);
	  break;

	case 's':		/* Save image number.  */
	  /* FIXME: Check if the input is an int  (Ole) */
	  if (save_image_given)
	    {
	      fprintf (stderr, "%s: `--save-image' (`-s') option redeclared\n", PACKAGE);
	      clear_args ();
	      cli_print_help ();
	      exit (1);
	    }
	  save_image_given = 1;
	  save_image_arg = atoi (optarg);
	  cli_save_image (save_image_arg, filename_arg);
	  break;

	case 't':		/* Save thumbnail number.  */
	  if (save_thumbnail_given)
	    {
	      fprintf (stderr, "%s: `--save-thumbnail' (`-t') option redeclared\n", PACKAGE);
	      clear_args ();
	      cli_print_help ();
	      exit (1);
	    }
	  save_thumbnail_given = 1;
	  save_thumbnail_arg = atoi (optarg);
	  cli_save_thumbnail (save_thumbnail_arg, filename_arg);
	  break;

	case 'd':		/* Delete image number.  */
	  if (delete_image_given)
	    {
	      fprintf (stderr, "%s: `--delete-image' (`-d') option redeclared\n", PACKAGE);
	      clear_args ();
	      cli_print_help ();
	      exit (1);
	    }
	  delete_image_given = 1;
	  delete_image_arg = atoi (optarg);
	  cli_delete_image (delete_image_arg);
	  break;

	case 'c':		/* Display camera summary.  */
	  cli_camera_summary ();	/* FIXME: Route */
	  break;

	case 'n':		/* Display number of pictures.  */
	  cli_display_numbers ();	/* FIXME: Fix it */
	  break;

	case 'l':		/* Save a live preview.  */
	  cli_save_live_preview (filename_arg);
	  break;

	case '?':		/* Invalid option.  */
	  /* `getopt_long' already printed
	     an error message.  */
	  exit (1);

	default:		/* bug: option not considered.  */
	  fprintf (stderr, "%s: option unknown: %c\n",
		   PACKAGE, c);
	  abort ();
	}
    }

  fflush (stdout);
  return 0;
}
