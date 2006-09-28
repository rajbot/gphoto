/* $Id$
 *
 * Copyright � 2002 Hans Ulrich Niedermann <hun@users.sourceforge.net>
 * Portions Copyright � 2002 Lutz M�ller <lutz@users.sourceforge.net>
 * Portions Copyright � 2002 FIXME
 *
 * This program is DEPRECATED.
 * Use print-camera-list (from packaging/generic/).
 * 
 * This program is free software; you can redistribute it and/or
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

#define GP_USB_HOTPLUG_SCRIPT "usbcam"
#define ARGV0 "print-usb-usermap"

#define HELP_TEXT \
"* DEPRECATED * DEPRECATED * DEPRECATED * DEPRECATED * DEPRECATED * DEPRECATED *\n" \
" If possible, please use print-camera-list (from packaging/generic/) instead\n" \
" of " ARGV0 ". If not, please help us fix print-camera-list.\n" \
"* DEPRECATED * DEPRECATED * DEPRECATED * DEPRECATED * DEPRECATED * DEPRECATED *\n" \
ARGV0 " - print usb.usermap file for linux-hotplug or FDI file for HAL\n" \
"\n" \
"Syntax:\n" \
"    " ARGV0 " [ --verbose ] [ --debug ] [ --fdi | <scriptname> ]\n" \
"\n" \
" --verbose   also print comments with camera model names\n" \
" --fdi       print FDI file for HAL\n" \
" --debug     print all debug output\n" \
"\n" \
ARGV0 " prints the lines for the usb.usermap file on stdout.\n" \
"All other messages are printed on stderr. In case of any error, the \n" \
"program aborts regardless of data printed on stdout and returns a non-zero\n" \
"status code.\n" \
"If no <scriptname> is given, " ARGV0 " uses the script name\n" \
"        \"" GP_USB_HOTPLUG_SCRIPT "\"." \
"If --fdi is given, " ARGV0 " generates a fdi file for HAL. Put it into\n" \
"/usr/share/hal/fdi/information/10freedesktop/10-camera-libgphoto2.fdi\n"

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-port-log.h>

#ifndef TRUE
#define TRUE  (0==0)
#endif
#ifndef FALSE
#define FALSE (0!=0)
#endif

#define GP_USB_HOTPLUG_MATCH_VENDOR_ID          0x0001
#define GP_USB_HOTPLUG_MATCH_PRODUCT_ID         0x0002

#define GP_USB_HOTPLUG_MATCH_DEV_CLASS          0x0080
#define GP_USB_HOTPLUG_MATCH_DEV_SUBCLASS       0x0100
#define GP_USB_HOTPLUG_MATCH_DEV_PROTOCOL       0x0200


#ifdef __GNUC__
#define CR(result) \
	do { \
		int r = (result); \
		if (r < 0) { \
			fprintf(stderr, ARGV0 ": " \
				"Fatal error running `%s'.\n" \
				"Aborting.\n", #result ); \
			return (r); \
		} \
	} while (0)
#else /* !__GNUC__ */
#define CR(result) \
	do { \
		int r = (result); \
		if (r < 0) { \
			fprintf(stderr, ARGV0 ": " \
				"Fatal error detected, aborting.\n"); \
			return (r); \
		} \
	} while (0)
#endif /* __GNUC__ */


/* print_usb_usermap
 *
 * Print out lines that can be included into usb.usermap 
 * - for all cams supported by our instance of libgphoto2.
 *
 * usb.usermap is a file used by
 * Linux Hotplug http://linux-hotplug.sourceforge.net/
 */

static int print_usb_usermap(const char *usermap_script, const int add_comments)
{
	int x, n;
	CameraAbilitiesList *al;
	CameraAbilities a;

	CR (gp_abilities_list_new (&al));
	CR (gp_abilities_list_load (al, NULL)); /* use NULL context */
	CR (n = gp_abilities_list_count (al));

        for (x = 0; x < n; x++) {
		int flags = 0;
		int class = 0, subclass = 0, proto = 0;
		int usb_vendor = 0, usb_product = 0;

		CR (gp_abilities_list_get_abilities (al, x, &a));

		if (!(a.port & GP_PORT_USB))
		    continue;

		if (a.usb_vendor) { /* usb product id may be 0! */
			class = 0;
			subclass = 0;
			proto = 0;
			flags = GP_USB_HOTPLUG_MATCH_VENDOR_ID | GP_USB_HOTPLUG_MATCH_PRODUCT_ID;
			usb_vendor = a.usb_vendor;
			usb_product = a.usb_product;
		} else if (a.usb_class) {
			class = a.usb_class;
			subclass = a.usb_subclass;
			proto = a.usb_protocol;
			flags = GP_USB_HOTPLUG_MATCH_DEV_CLASS;
			if (subclass != -1)
			    flags |= GP_USB_HOTPLUG_MATCH_DEV_SUBCLASS;
			else
			    subclass = 0;
			if (proto != -1)
			    flags |= GP_USB_HOTPLUG_MATCH_DEV_PROTOCOL;
			else
			    proto = 0;
			usb_vendor = 0;
			usb_product = 0;
		} else {
			flags = 0;
		}

		if (flags != 0) {
			printf ("# %s\n", 
				a.model);
			/* The first 3 lone bytes are the device class.
			 * the second 3 lone bytes are the interface class.
			 * for PTP we want the interface class.
			 */
			printf ("%-20s "
				"0x%04x      0x%04x   0x%04x    0x0000       "
				"0x0000      0x00         0x00            "
				"0x00            0x%02x            0x%02x               "
				"0x%02x               0x00000000\n",
				usermap_script, flags, 
				a.usb_vendor, a.usb_product,
				class, subclass, proto);
		} else {
			fputs ("Error: Neither vendor/product nor class/subclass matched\n", stderr);
			return 2;
		}
        }
	CR (gp_abilities_list_free (al));

        return (0);
}


/* print_fdi_map
 *
 * Print FDI Device Information file for HAL with information on 
 * all cams supported by our instance of libgphoto2.
 *
 * For specs, look around on http://freedesktop.org/ and search
 * for FDI on the HAL pages.
 */

static int print_fdi_map(const int add_comments, const char *argv0)
{
	int x, n;
	CameraAbilitiesList *al;
	CameraAbilities a;

	CR (gp_abilities_list_new (&al));
	CR (gp_abilities_list_load (al, NULL)); /* use NULL context */
	CR (n = gp_abilities_list_count (al));

	printf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> <!-- -*- SGML -*- -->\n");
	printf("<!-- This file was generated by %s - - fdi -->\n",
		(argv0!=NULL)?argv0:"libgphoto2 " ARGV0);
	printf("<deviceinfo version=\"0.2\">\n");
	printf(" <device>\n");
	printf("  <match key=\"info.bus\" string=\"usb\">\n");


        for (x = 0; x < n; x++) {
		char	*s, *d, model[256];

		CR (gp_abilities_list_get_abilities (al, x, &a));

		if (!(a.port & GP_PORT_USB))
		    continue;

		s = a.model; d = model;
		while (*s) {
		    if (*s == '&') {
			strcpy(d,"&amp;");
			d += strlen(d);
		    } else {
		    	*d++ = *s;
		    }
		    s++;
		}
		*d = '\0';
		if (a.usb_vendor) { /* usb product id might be 0! */
			printf("   <match key=\"usb.vendor_id\" int=\"%d\">\n", a.usb_vendor);
			printf("    <match key=\"usb.product_id\" int=\"%d\">\n", a.usb_product);
			printf("     <merge key=\"info.category\" type=\"string\">camera</merge>\n");
			printf("     <append key=\"info.capabilities\" type=\"strlist\">camera</append>\n");

			/* HACK alert ... but the HAL / gnome-volume-manager guys want that */
			if (NULL!=strstr(a.library,"ptp"))
				printf("     <merge key=\"camera.access_method\" type=\"string\">ptp</merge>\n");
			else
				printf("     <merge key=\"camera.access_method\" type=\"string\">proprietary</merge>\n");
			printf("     <merge key=\"camera.libgphoto2.name\" type=\"string\">%s</merge>\n", model);
			printf("     <merge key=\"camera.libgphoto2.support\" type=\"bool\">true</merge>\n");
			printf("    </match>\n");
			printf("   </match>\n");

		} else if (a.usb_class) {
			printf("   <match key=\"usb.interface.class\" int=\"%d\">\n", a.usb_class);
			printf("    <match key=\"usb.interface.subclass\" int=\"%d\">\n", a.usb_subclass);
			printf("     <match key=\"usb.interface.protocol\" int=\"%d\">\n", a.usb_protocol);
			printf("      <merge key=\"info.category\" type=\"string\">camera</merge>\n");
			printf("      <append key=\"info.capabilities\" type=\"strlist\">camera</append>\n");
			if (a.usb_class == 6) {
				printf("      <merge key=\"camera.access_method\" type=\"string\">ptp</merge>\n");
			} else {
				if (a.usb_class == 8) {
					printf("      <merge key=\"camera.access_method\" type=\"string\">storage</merge>\n");
				} else {
					printf("      <merge key=\"camera.access_method\" type=\"string\">proprietary</merge>\n");
				}
			}
			printf("      <merge key=\"camera.libgphoto2.name\" type=\"string\">%s</merge>\n", model);
			printf("      <merge key=\"camera.libgphoto2.support\" type=\"bool\">true</merge>\n");
			printf("     </match>\n");
			printf("    </match>\n");
			printf("   </match>\n");
		}
        }
	printf("  </match>\n");
	printf(" </device>\n");
	printf("</deviceinfo>\n");
	CR (gp_abilities_list_free (al));
        return (0);
}


/* time zero for debug log time stamps */
struct timeval glob_tv_zero = { 0, 0 };

static void
debug_func (GPLogLevel level, const char *domain, const char *format,
	    va_list args, void *data)
{
	struct timeval tv;
	gettimeofday (&tv,NULL);
	fprintf (stderr, "%li.%06li %s(%i): ", 
		 tv.tv_sec - glob_tv_zero.tv_sec, 
		 (1000000 + tv.tv_usec - glob_tv_zero.tv_usec) % 1000000,
		 domain, level);
	vfprintf (stderr, format, args);
	fprintf (stderr, "\n");
}


int main(int argc, char *argv[])
{
	char *usermap_script = NULL; /* script name to use */
	int add_comments = FALSE;    /* whether to add cam model as a comment */
	int debug_mode = FALSE;      /* whether we should output debug messages */
	int fdi_mode = FALSE;

	int i;

	/* check command line arguments */
	for (i=1; i<argc; i++) {
		if (0 == strcmp(argv[i], "--verbose")) {
			if (add_comments) {
				fprintf(stderr, "Error: duplicate parameter: option \"%s\"\n", argv[i]);
				return 1;
			}
			add_comments = TRUE;
		} else if (0 == strcmp(argv[i], "--debug")) {
			if (debug_mode) {
				fprintf(stderr, "Error: duplicate parameter: option \"%s\"\n", argv[i]);
				return 1;
			}
			debug_mode = TRUE;
			/* now is time zero for debug log time stamps */
			gettimeofday (&glob_tv_zero, NULL);
			gp_log_add_func (GP_LOG_ALL, debug_func, NULL);
		} else if (0 == strcmp(argv[i], "--fdi")) {
			if (fdi_mode) {
				fprintf(stderr, "Error: duplicate parameter: option \"%s\"\n", argv[i]);
				return 1;
			}
			fdi_mode = TRUE;
		} else if (0 == strcmp(argv[i], "--help")) {
			puts(HELP_TEXT);
			return 0;
		} else {
			if (usermap_script != NULL) {
				fprintf(stderr, "Error: duplicate script parameter: \"%s\"\n", argv[i]);
				return 1;
			}
			/* assume script name */
			usermap_script = argv[i];
		}
	}

	if (NULL == usermap_script) {
		usermap_script = GP_USB_HOTPLUG_SCRIPT;
	}

	if (fdi_mode)
		return print_fdi_map(add_comments, argv[0]);
	else
		return print_usb_usermap(usermap_script, add_comments);
}

/*
 * Local Variables:
 * c-file-style:"linux"
 * indent-tabs-mode:t
 * End:
 */
