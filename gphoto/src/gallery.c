/* $Id$
 *
 * gPhoto - free digital camera utility - http://www.gphoto.org/

   HTML Gallery Engine for generating a web gallery of your images
   from within gPhoto. Can be used with any camera library, or even
   with "browse directory" to create your html gallery...

	Gallery tags:
		GALLERY_NAME
		GALLERY_INDEX

		DATE

		THUMBNAIL
		THUMBNAIL_FILENAME
		THUMBNAIL_NUMBER

		PICTURE
		PICTURE_FILENAME
		PICTURE_NUMBER

		PICTURE_NEXT
		PICTURE_PREVIOUS

 * Copyright (C) 1999  Scott Fritzinger <scottf@scs.unr.edu>
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
 * Revision 1.12  1999/06/30 00:13:16  scottf
 * updated the "get blank index" icon to be a little more consistent
 * added gtk_directory_selection_new() and related functions
 * 	(possible new GTK widget?)
 * updated priority to go back to min priority when execute_program is
 * 	called
 * other misc fixes.
 *
 * Revision 1.11  1999/06/29 19:51:02  scottf
 * label rewording
 * added check for "%s" in the post-process command-line
 * fixed some gallery issues so that the $PWD is displayed in the label
 * removed the ugly scrollbars in the directory file_selection boxes.
 * other misc fixes as well. (good day! :)
 *
 * Revision 1.10  1999/06/29 01:55:36  scottf
 * source code tidying. moved browse_ callbacks to callbacks.c but left
 * url_send_browse in util (could be very handy).
 * also removed some toolbar icons (plug-ins) just to try to remove a little
 * clutter.
 *
 * Revision 1.9  1999/06/21 18:26:43  ole
 * Removed deactivate_button(browse_button);
 *
 * Revision 1.8  1999/06/21 18:04:12  ole
 * 1999-06-21  Ole Aamot  <oleaa@ifi.uio.no>
 *
 * 	* callbacks.c: changed activate_button(..) a bit
 * 	* developer_dialog.c: added a clickable webpage button. :o)
 * 	* gallery.c: added browse_gallery();
 * 	* main.c: added browse_gphoto();
 * 	* menu.c: added menu-links to www.gphoto.org (loads in a BROWSER)
 * 	* toolbar.c: added/changed icons (by tigert) for the plugins
 * 	* util.c: added url_send_browser(..), and browse_* web routines.
 *
 * Revision 1.7  1999/06/19 15:32:57  ole
 * Generated SUPPORTED from http://www.gphoto.org/cameras.php3
 *
 * Revision 1.6  1999/06/15 19:44:32  scottf
 * various updates to headers and sources
 * added constrain proportions to resize
 *
 * Revision 1.5  1999/06/15 16:05:04  scottf
 * removed the #ifdef's for gtk < 1.2
 * windows show in middle of screen now instead of random places :)
 *
 * Revision 1.4  1999/06/10 19:53:32  scottf
 * Updated the functions wrappers to check for errors everywhere
 * Updated the gallery to display an error if the thumbnails can't
 * 	be saved.
 * Updated so that now, the last directory you selected is the one
 * 	"working directory" in most fileselection boxes
 * 	(reduces mouse clicks :)
 *
 * Revision 1.3  1999/06/10 16:38:54  scottf
 * Updated the callbacks.h file
 * FIxed the gallery numbering bug (again)
 * Little fixes here and there
 *
 * Revision 1.2  1999/06/08 19:08:10  scottf
 * Fixed several gallery.c bugs, and improved it a bit.
 * Added free_image and save_image to util.c
 *
 * Revision 1.1.1.1  1999/05/27 18:32:07  scottf
 * gPhoto- digital camera utility
 *
 * Revision 1.11  1999/05/08 13:31:10  ole
 * Changed paths from /usr/share/... to /usr/local/share/...
 *
 */

#include "main.h"
#include "gphoto.h"
#include "util.h"
#include "callbacks.h"
#include "gallery.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>  
#include <sys/dir.h>

extern GtkWidget *index_window;
/* extern GtkWidget *browse_button; */

extern struct ImageInfo Thumbnails;
extern struct ImageInfo Images;
extern struct _Camera *Camera;
extern char   *filesel_cwd;

/* initialize some tags */
char gallery_name[256];
char gallery_index[128];
char date[32];
char thumbnail[256];
char thumbnail_filename[128];
char thumbnail_number[5];
char picture[256];
char picture_filename[128];
char picture_number[5];
char picture_next[128];
char picture_previous[128];

void gallery_change_dir(GtkWidget *widget, GtkWidget *label) {

	GtkWidget *filesel;

        filesel = gtk_directory_selection_new("Select an Output Directory...");
	gtk_window_set_position (GTK_WINDOW (filesel), GTK_WIN_POS_CENTER);
	gtk_widget_show(filesel);

        if (wait_for_hide(filesel,
           GTK_FILE_SELECTION(filesel)->ok_button,
           GTK_FILE_SELECTION(filesel)->cancel_button) == 0) {
		return;
	}

	gtk_label_set(GTK_LABEL(label), strdup(
	gtk_file_selection_get_filename(GTK_FILE_SELECTION(filesel))));
	gtk_object_set_data(GTK_OBJECT(label), "dir", strdup(
	   gtk_file_selection_get_filename(GTK_FILE_SELECTION(filesel))));
	sprintf(filesel_cwd, "%s", gtk_file_selection_get_filename(
		GTK_FILE_SELECTION(filesel)));
	gtk_widget_destroy(filesel);
}

void gallery_parse_tags(char *dest, char *source) {

	char fname[1024];
	sprintf(fname,
"sed 's/#GALLERY_NAME#/%s/; \
s/#GALLERY_INDEX#/%s/; \
s/#DATE#/%s/; \
s/#THUMBNAIL#/%s/; \
s/#THUMBNAIL_FILENAME#/%s/; \
s/#THUMBNAIL_NUMBER#/%s/; \
s/#PICTURE#/%s/; \
s/#PICTURE_FILENAME#/%s/; \
s/#PICTURE_NUMBER#/%s/; \
s/#PICTURE_NEXT#/%s/; \
s/#PICTURE_PREVIOUS#/%s/;' %s >> %s",
gallery_name, gallery_index, date, thumbnail, thumbnail_filename,
thumbnail_number, picture, picture_filename, picture_number,
picture_next, picture_previous, source, dest);

/*	printf("%s", fname);*/
	system(fname);
}

void gallery_main() {

	DIR *dir, *testdir;
	struct dirent *file;
	time_t loctime;

	int i=0, j=0, num_selected = 0;
	char outputdir[1024], theme[32];
	char filename[1024], filename2[1024], cp[1024], error[32], statmsg[128];
	struct Image *im;

	GList *dlist;

	GtkWidget *dialog, *list, *list_item, *scrwin, *obutton, *cbutton;
	GtkWidget *galentry;
	GtkWidget *label, *dirlabel, *dirbutton, *hbox, *hseparator;

	struct ImageInfo *node = &Thumbnails;
	struct ImageInfo *node_image;

	if (Thumbnails.next == NULL) {
		error_dialog(
"Please retrieve the index first,
and select the images to include
in the gallery by clicking on them.
Then, re-run the HTML Gallery.");
		return;
	}

	/* Create the file selection to change directory */

	i = 0;
	while (node->next != NULL) {
		node = node->next;
		if (GTK_TOGGLE_BUTTON(node->button)->active)
			num_selected++;
	}
	if (num_selected == 0) {
		error_dialog(
			"You need to select images to add\nto the gallery.");
		return;
	}

	dialog = gtk_dialog_new();
	gtk_widget_set_usize(dialog, 400, 300);
	gtk_container_border_width(GTK_CONTAINER(dialog), 5);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_widget_set_usize(hbox, 390, 25);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox,
		FALSE, FALSE, 0);

	label = gtk_label_new("Gallery Name:");
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	galentry = gtk_entry_new();
	gtk_widget_show(galentry);
	gtk_box_pack_start_defaults(GTK_BOX(hbox), galentry);

	hseparator = gtk_hseparator_new();
	gtk_widget_show(hseparator);
	gtk_widget_set_usize(hseparator, 390, 5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hseparator,
		FALSE, FALSE, 0);

	label = gtk_label_new("Theme:");
	gtk_widget_show(label);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_widget_set_usize(label, 390, 25);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label,
		FALSE, FALSE, 0);

	list = gtk_list_new();
        gtk_widget_show(list);
        gtk_list_set_selection_mode (GTK_LIST(list),GTK_SELECTION_SINGLE);

	dir = opendir("/usr/local/share/gphoto/gallery");
	if (dir == NULL) {
		error_dialog(
"HTML gallery themes do not exist at
/usr/local/share/gphoto/gallery
Please install/move gallery themes there.");
		return;
	}

	file = readdir(dir);
	while (file != NULL) {
		sprintf(filename, "/usr/local/share/gphoto/gallery/%s",
			file->d_name);
		testdir = opendir(filename);
                if ((strcmp(file->d_name, ".") != 0) &&
                    (strcmp(file->d_name, "..") != 0) &&
		    (testdir != NULL)) {
			closedir(testdir);
	                list_item = gtk_list_item_new_with_label(file->d_name);
	                gtk_widget_show(list_item);
        	        gtk_container_add(GTK_CONTAINER(list), list_item);
                	gtk_object_set_data(GTK_OBJECT(list_item),"theme",
                                        file->d_name);
		}
		file = readdir(dir);
	}

        scrwin = gtk_scrolled_window_new(NULL, NULL);
        gtk_widget_show(scrwin);
        gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scrwin), list);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(index_window),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		scrwin);

	hseparator = gtk_hseparator_new();
	gtk_widget_show(hseparator);
	gtk_widget_set_usize(hseparator, 390, 5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hseparator,
		FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_widget_set_usize(hbox, 390, 25);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox,
		FALSE, FALSE, 0);	

	label = gtk_label_new("Output Directory:");
	gtk_widget_show(label);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	dirlabel = gtk_label_new(filesel_cwd);
	gtk_widget_show(dirlabel);
	gtk_label_set_justify(GTK_LABEL(dirlabel), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start_defaults(GTK_BOX(hbox), dirlabel);
	gtk_object_set_data(GTK_OBJECT(dirlabel), "dir", filesel_cwd);

	dirbutton = gtk_button_new_with_label("Change...");
	gtk_widget_show(dirbutton);
	gtk_signal_connect(GTK_OBJECT(dirbutton), "clicked",
		GTK_SIGNAL_FUNC(gallery_change_dir),dirlabel);
	gtk_box_pack_end(GTK_BOX(hbox), dirbutton, FALSE, FALSE, 0);

	obutton = gtk_button_new_with_label("Create");
	gtk_widget_show(obutton);
        GTK_WIDGET_SET_FLAGS (obutton, GTK_CAN_DEFAULT);
	gtk_box_pack_start_defaults(
		GTK_BOX(GTK_DIALOG(dialog)->action_area), obutton);

	cbutton = gtk_button_new_with_label("Cancel");
	gtk_widget_show(cbutton);
        GTK_WIDGET_SET_FLAGS (cbutton, GTK_CAN_DEFAULT);
	gtk_box_pack_start_defaults(
		GTK_BOX(GTK_DIALOG(dialog)->action_area), cbutton);
	gtk_widget_show(dialog);
	gtk_widget_grab_default(obutton);
	
	/* wait until the dialog is closed */
        if (wait_for_hide(dialog, obutton, cbutton) == 0)
                return;

	dlist = GTK_LIST(list)->selection;
	if (!dlist) {
		error_dialog("You must select a theme!");
		return;
	}

	strcpy(cp, "");
	sprintf(theme, "%s",(char*)gtk_object_get_data(GTK_OBJECT(dlist->data),
		"theme"));
	sprintf(gallery_name,"%s",
		gtk_entry_get_text(GTK_ENTRY(galentry)));
	strcpy(gallery_index, "index.html");
	loctime = time(&loctime);
        sprintf(date, "%s", ctime(&loctime));
	date[24] = '\0';
 	sprintf(outputdir,"%s",
		(char*)gtk_object_get_data(GTK_OBJECT(dirlabel),"dir"));

	sprintf(filename, "/usr/local/share/gphoto/gallery/%s",
		theme);
	dir = opendir(filename);
	file = readdir(dir);
	while (file != NULL) {
		if ((strcmp(file->d_name, "index_top.html") != 0 ) &&
		    (strcmp(file->d_name, "index_bottom.html") != 0 ) &&
		    (strcmp(file->d_name, "picture.html") != 0 ) &&
		    (strcmp(file->d_name, "thumbnail.html") != 0 ) &&
		    (strcmp(file->d_name, ".") != 0 ) &&
		    (strcmp(file->d_name, "..") != 0 )) {
			sprintf(cp, "cp %s/%s %s", filename, file->d_name,
						   outputdir);
			system(cp);
		}
		file = readdir(dir);
	}	
	closedir(dir);
	sprintf(filename, "%sindex.html", outputdir);	
	sprintf(cp, "echo \" \" > %s", filename);
	system(cp);
	sprintf(filename2,
		"/usr/local/share/gphoto/gallery/%s/index_top.html",
		theme);	
	gallery_parse_tags(filename, filename2);
	sprintf(cp, "echo \"<table border=1><tr>\" >> %s", filename);
	system(cp);
	node = &Thumbnails;
	while (node->next != NULL) {
		int loaded;	
		int picnum;	
		node = node->next;
		sprintf(filename, "%sindex.html", outputdir);	
		if (GTK_TOGGLE_BUTTON(node->button)->active) {
			if ((i%5==0) && (i!=0)) {
				sprintf(cp, "echo \"</tr><tr>\" >> %s",
					filename);
				system(cp);
			}
		/* Get the current thumbnail */
			sprintf(cp, "Getting Thumbnail #%i...", j+1);
			update_status(cp);
			if ((im = (*Camera->get_picture)(j+1, 1))==0) {
				sprintf(error, "Could not retrieve #%i",
					j+1);
				error_dialog(error);
				sprintf(thumbnail, "Not Available");
				sprintf(thumbnail_filename, "");
				sprintf(thumbnail_number, "%03i", i+1);
			} else {
				sprintf(thumbnail, 
	"<a href=\"picture-%03i.html\"><img src=\"thumbnail-%03i.%s\"><\\/a>",
					i+1, i+1, im->image_type);
				sprintf(thumbnail_filename, "thumbnail-%03i.%s",
					i+1, im->image_type);
				sprintf(thumbnail_number, "%03i", i+1);
				sprintf(filename2, "%s%s", outputdir,
					thumbnail_filename);
				save_image(filename2, im);
				free_image(im);
			}
			/* Get the current image */
			sprintf(cp, "Getting Image #%i...", j+1);
			update_status(cp);
			
			if ((im = (*Camera->get_picture)(j+1, 0))==0) {
				sprintf(error, "Could not retrieve #%i", j+1);
				error_dialog(error);	
				sprintf(picture, "Not Available");
				sprintf(picture_filename, "");
				sprintf(picture_number, "%03i", i+1);}
			   else {
				sprintf(picture, "<img src=\"picture-%03i.%s\">",
                                	i+1, im->image_type);
                        	sprintf(picture_filename, "picture-%03i.%s",
                                	i+1, im->image_type);
				sprintf(picture_number, "%03i", i+1);
				sprintf(filename2, "%s%s", outputdir,
					picture_filename);
				save_image(filename2, im);
				free_image(im);
			}
			if (i+1 == num_selected)
				strcpy(picture_next, gallery_index);
			   else	
				sprintf(picture_next, "picture-%03i.html",
					i+2);
			if (i == 0)
				strcpy(picture_previous, gallery_index);
			   else
				sprintf(picture_previous, "picture-%03i.html",
					i);
			sprintf(cp, "echo \"<td>\" >> %s", filename);
			system(cp);

			sprintf(filename2,
				"/usr/local/share/gphoto/gallery/%s/thumbnail.html",
				theme);
			gallery_parse_tags(filename, filename2);
			sprintf(cp, "echo \"</td>\" >> %s", filename);
			system(cp);

			sprintf(filename, "%spicture-%03i.html",
				outputdir, i+1);
			sprintf(cp, "echo \" \" > %s", filename);
			system(cp);
		
			sprintf(filename2,
				"/usr/local/share/gphoto/gallery/%s/picture.html",
				theme);
				gallery_parse_tags(filename, filename2);
			i++;
		}
		j++;
	}
	sprintf(filename, "%sindex.html", outputdir);	
	sprintf(cp, "echo \"</tr></table>\" >> %s", filename);
	system(cp);
	sprintf(filename2,
		"/usr/local/share/gphoto/gallery/%s/index_bottom.html",
		theme);			
	gallery_parse_tags(filename, filename2);
	gtk_widget_destroy(dialog);
	browse_gallery();
	sprintf(statmsg, "Loaded file:%sindex.html in %s", filesel_cwd, BROWSER);
	update_status(statmsg);
/*	activate_button(browse_button); */
}
