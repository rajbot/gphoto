/* gPhoto - free digital camera utility - http://www.gphoto.org/
 *
 * Copyright (C) 1999  The gPhoto developers  <gphoto-devel@gphoto.org>
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
 */

#include "gphoto.h"
#include "main.h"
#include "util.h"
#include "callbacks.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include "post_processing_on.xpm"
#include "post_processing_off.xpm"

extern struct ImageInfo Thumbnails;
extern struct ImageInfo Images;
extern struct _Camera *Camera;

extern char	  camera_model[];
extern char	  serial_port[];
extern int	  post_process;
extern char	  post_process_script[];
extern GtkWidget *post_process_pixmap;

/* Search the image_info tags for "name", return its value (string) */
char* find_tag(struct Image *im, char* name) {

	int i;

	if (im==NULL) return(NULL);
	if (im->image_info==NULL) return(NULL);

	for (i=0;i<im->image_info_size;i+=2)
		if (!strncmp((im->image_info)[i],name,100))
			return((im->image_info)[i+1]);

	return(NULL); /* Nothing Found */
};

void set_camera (char *model) {

	int i=0;

	while (strlen(cameras[i].name) > 0) {
		if (strcmp(model, cameras[i].name) == 0) {
			gtk_label_set(GTK_LABEL(library_name),
					cameras[i].name);
			Camera = cameras[i].library;
			if ((*Camera->initialize)() != 0) {
				return;
			}
		}
		i++;
	}
	error_dialog("Could not initialize the library.");
}

void configure_call() {

	if (Camera != NULL) {
		if ((*Camera->configure)() == 0) {
	  		error_dialog("No configuration options.");
		}
	} else {
	  	error_dialog("No configuration options.");
	}
}

void takepicture_call() {

	char status[256];
	int picNum;

	update_status("Taking picture...");
printf("Calling the take_picture function!");
	picNum = (*Camera->take_picture)();

	if (picNum == 0) {
		error_dialog("Could not take a picture.");
		return;
	}

	sprintf(status, "New picture is #%i", picNum);

	error_dialog(status);
	update_status("Done.");
}

void del_pics (GtkWidget *dialog, GtkObject *button) {

        int i=1;
	char error[32];

        struct ImageInfo *node = &Thumbnails;

        gtk_widget_hide(dialog);
        update_status("Deleting selected pictures...");

        /* delete from camera... */
        node = node->next;  /* Point at first thumbnail */
        while (node != NULL) {
            	if (GTK_TOGGLE_BUTTON(node->button)->active) {
			node = node->next;
                	if ((*Camera->delete_picture)(i) != 0) {
				remove_thumbnail(i);
        	        	i--;}
			   else {
				sprintf(error, 
				"Could not delete #%i", i);
				error_dialog(error);
			}}
		   else {
			node = node->next;
		}
		i++;
        }
        gtk_widget_destroy(dialog);
        update_status("Done.");
}

void del_dialog () {

	/*
	   Shows the delete confirmation dialog
	*/

	int nothing_selected = 1;
	GtkWidget *dialog, *button, *label;

	struct ImageInfo *node = &Thumbnails;
	while (node->next != NULL) {
		node = node->next;
		if (GTK_TOGGLE_BUTTON(node->button)->active)
			nothing_selected = 0;
	}
	if (nothing_selected) {
		update_status("Nothing selected to be deleted.");
		return;
	}

	dialog = gtk_dialog_new();

	label = gtk_label_new("Are you sure you want to DELETE the selected images?");
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label,
			   FALSE, FALSE, 0);

	button = gtk_button_new_with_label("Yes");
	gtk_widget_show(button);
	gtk_signal_connect_object(GTK_OBJECT(button), "clicked", 
				  GTK_SIGNAL_FUNC(del_pics), 
				  GTK_OBJECT(dialog));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
			   button, FALSE, FALSE, 0);
	button = gtk_button_new_with_label("No");
	gtk_widget_show(button);
	gtk_signal_connect_object(GTK_OBJECT(button), "clicked", 
				  GTK_SIGNAL_FUNC(gtk_widget_destroy), 
				  GTK_OBJECT(dialog));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
			   button, FALSE, FALSE, 0);

	gtk_widget_show(dialog);
}

void savepictodisk (int picNum, int thumbnail, char *prefix) {

	/* Saves picture (thumbnail == 0) or thumbnail (thumbnail == 1)
	 * #picNum to disk as prefix.(returned image extension)
	 */

	FILE *fp;
	struct Image *im = NULL; 
	char fname[1024], error[32], process[1024];

	if ((im = (*Camera->get_picture)(picNum, thumbnail)) == 0) {
		sprintf(error, "Could not save #%i", picNum);
		error_dialog(error);
		return;
	}
	sprintf(fname, "%s.%s", prefix, im->image_type);
	save_image(fname, im);
	if (post_process) {
		sprintf(process, post_process_script, fname);
		system(process);
	}
	free_image(im);
}

char saveselectedtodisk_dir[1024];

void saveselectedtodisk (GtkWidget *widget, char *type) {

	/* Odd workaround:
	 * if type == "tn" (thumbnails/no directory needed)
	 * or type == "in" (images/no directory needed)
	 * then it is assume that saveselectedtodisk_dir contains
	 * the path to where we want to save. this allows the batch
	 * saving of both images and thumbnails without prompting
	 * for an output directory for thumbnails, and then images.
	 * if type == "t", selected thumbnails are saved.
	 * if type == "i", selected images are saved.
	 */

	int i = 0, pic = 1;
	char fname[1024], status[32];
	char *filesel_dir, *filesel_prefix;

	struct ImageInfo *node = &Thumbnails;

	GtkWidget *filesel, *label;
	GtkWidget *entry;
	GSList *group;

	if ((strcmp("tn", type) != 0) && (strcmp("in", type) != 0)) {

		/* Get an output directory */

		filesel = gtk_file_selection_new(
				"Select a directory to store the images...");
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(filesel),
			filesel_cwd);
		gtk_widget_hide(GTK_FILE_SELECTION(filesel)->file_list);
		gtk_widget_hide(GTK_FILE_SELECTION(filesel)->selection_text);
		gtk_widget_hide(GTK_FILE_SELECTION(filesel)->selection_entry);

		label = gtk_label_new("Enter the file prefix for the images:");
		gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
		gtk_widget_show(label);
		gtk_box_pack_start_defaults(GTK_BOX(
			GTK_FILE_SELECTION(filesel)->main_vbox), label);
		gtk_box_reorder_child(GTK_BOX(
			GTK_FILE_SELECTION(filesel)->main_vbox), label, 5);

	        entry = gtk_entry_new();
	        gtk_widget_show(entry);
	        gtk_entry_set_max_length(GTK_ENTRY(entry), 25);
		gtk_box_pack_start_defaults(GTK_BOX(
			GTK_FILE_SELECTION(filesel)->main_vbox), entry);
		gtk_box_reorder_child(GTK_BOX(
			GTK_FILE_SELECTION(filesel)->main_vbox), entry, 6);
	
		/* if they clicked cancel, return  ------------- */
		if (wait_for_hide(filesel, GTK_FILE_SELECTION(filesel)->ok_button, 
		    GTK_FILE_SELECTION(filesel)->cancel_button) == 0)
			return;
	        /* --------------------------------------------- */

		filesel_dir = gtk_file_selection_get_filename(
				GTK_FILE_SELECTION(filesel));
	        strcpy(filesel_cwd, filesel_dir);
		filesel_prefix = gtk_entry_get_text(GTK_ENTRY(entry));
		sprintf(saveselectedtodisk_dir, "%s%s", filesel_dir,
			filesel_prefix);
	}

	while (node->next != NULL) {
                node = node->next; i++;
                if (GTK_TOGGLE_BUTTON(node->button)->active) {
			if ((strcmp("i", type)==0)||(strcmp("in", type)==0)) {
				sprintf(status, "Saving Image #%i...", i);
				update_status(status);
				sprintf(fname, "%s-%i",
					saveselectedtodisk_dir, pic);
				savepictodisk(i, 0, fname); }
			   else {
				sprintf(status, "Saving Thumbnail #%i...", i);
				update_status(status);
				sprintf(fname, "%s-thumbnail-%i",
					saveselectedtodisk_dir, pic);
				savepictodisk(i, 1, fname);
			}
			pic++;
		}
	}
	sprintf(fname, "Done. Images saved in %s", filesel_dir);
	update_status(fname);
}

void appendpic (int picNum, int thumbnail, int fromCamera, char *fileName) {

	int w, h;
	char fname[15], error[32], process[1024],
	     imagename[1024],*openName;
	
	GtkWidget *scrwin, *label;
	GdkPixmap *pixmap;
	struct Image *im;

	struct ImageInfo *node = &Images;

	while (node->next != NULL)
		node = node->next;

	node->next = malloc(sizeof(struct ImageInfo));
	node = node->next; node->next = NULL;

	if (fromCamera) {
		if ((im = (*Camera->get_picture)(picNum, thumbnail))==0) {
			sprintf(error, "Could not retrieve #%i", picNum);
			error_dialog(error);
			return;
		}
		node->imlibimage =  gdk_imlib_load_image_mem(im->image,
							im->image_size);
		free_image(im);
		if (post_process) {
			sprintf(imagename, "%s/gphoto-image-%i.jpg",
				gphotoDir, picNum);
			sprintf(process, post_process_script, imagename);
			gdk_imlib_save_image(node->imlibimage,
				imagename, NULL);
			gdk_imlib_kill_image(node->imlibimage);
			system(process);
			node->imlibimage = gdk_imlib_load_image(imagename);
			remove(imagename);
		}}
	   else
		node->imlibimage = gdk_imlib_load_image(fileName);
	w = node->imlibimage->rgb_width;
        h = node->imlibimage->rgb_height;
        gdk_imlib_render(node->imlibimage, w, h);
        pixmap = gdk_imlib_move_image(node->imlibimage);
        node->image = gtk_pixmap_new(pixmap, NULL);
	gtk_widget_show(node->image);

	/* Append to notebook -------------------------------- */
	sprintf(fname, "%i", picNum);
	node->info = strdup(fname);
	if (fromCamera) {
		sprintf(fname, "Image %i", picNum);
		label = gtk_label_new(fname);}
	   else {
		openName = strrchr(fileName, (int)'/');
		label = gtk_label_new(&openName[1]);
	}
        node->page = gtk_vbox_new(FALSE,0);
	gtk_widget_show(node->page);
        gtk_notebook_append_page (GTK_NOTEBOOK(notebook), node->page,
                                  label);
        scrwin = gtk_scrolled_window_new(NULL,NULL);
        gtk_widget_show(scrwin);
	gtk_container_add(GTK_CONTAINER(node->page), scrwin);

#ifndef GTK_HAVE_FEATURES_1_1_4
	gtk_container_add(GTK_CONTAINER(scrwin), node->image);
#else
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrwin),
					      node->image);
#endif
}

/* GTK Functions ---------------------------------------------
   ----------------------------------------------------------- */

void destroy (GtkWidget *widget, gpointer data) {

        gtk_main_quit ();
}

gint delete_event (GtkWidget *widget, GdkEvent *event, gpointer data) {

        destroy(widget, data);
        return (FALSE);
}


/* Callbacks -------------------------------------------------
   ----------------------------------------------------------- */

void port_dialog() {

	GtkWidget *dialog, *label, *button, *cbutton, *toggle;
	GtkWidget *port0, *port1, *port2, *port3, *other, *ent_other;
	GtkWidget *scrwin, *list, *list_item;
	GtkWidget *hbox, *vbox, *vseparator;
	GSList *group;
	GList *dlist;
	GtkObject *olist_item;

	FILE *conf;
	char serial_port_prefix[20], tempstring[20];
	int i=0;

#ifdef linux
        sprintf(serial_port_prefix, "/dev/ttyS");
#elif defined(__FreeBSD__) || defined(__NetBSD__)
        sprintf(serial_port_prefix, "/dev/tty0");
#else
        sprintf(serial_port_prefix, "/dev/tty0");
#endif

	dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog), "Select model/port...");
	gtk_container_border_width(GTK_CONTAINER(dialog), 5);
	gtk_widget_set_usize(dialog, 400, 300);

	/* Box going across the dialog... */
	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),
					hbox);

	/* For the camera selection... */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_box_pack_start_defaults(GTK_BOX(hbox), vbox);

	label = gtk_label_new("Camera Model:");
	gtk_widget_show(label);
        gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

	list = gtk_list_new();
	gtk_widget_show(list);
	gtk_list_set_selection_mode (GTK_LIST(list),GTK_SELECTION_SINGLE);

 	scrwin = gtk_scrolled_window_new(NULL, NULL);
 	gtk_widget_show(scrwin);
#ifndef GTK_HAVE_FEATURES_1_1_4
 	gtk_container_add(GTK_CONTAINER(scrwin), list);
#else
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrwin),
					      list);
#endif
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(index_window),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC); 
        gtk_box_pack_start_defaults(GTK_BOX(vbox),scrwin);

	while (strlen(cameras[i].name) > 0) {
		list_item = gtk_list_item_new_with_label(cameras[i].name);
		gtk_widget_show(list_item);
		if (strcmp(cameras[i].name, camera_model) == 0)
			gtk_widget_set_state(list_item,GTK_STATE_ACTIVE);
		gtk_container_add(GTK_CONTAINER(list), list_item);
		gtk_object_set_data(GTK_OBJECT(list_item),"model",
					cameras[i].name);
		i++;
	}

	vseparator = gtk_vseparator_new();
	gtk_widget_show(vseparator);
	gtk_box_pack_start(GTK_BOX(hbox), vseparator, FALSE, FALSE, 0);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

	label = gtk_label_new("Port:");
	gtk_widget_show(label);
	port0 = gtk_radio_button_new_with_label(NULL, "Serial Port 1");
        gtk_widget_show(port0);
	gtk_button_clicked(GTK_BUTTON(port0));
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(port0));
        port1 = gtk_radio_button_new_with_label(group, "Serial Port 2");
        gtk_widget_show(port1);
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(port1));
        port2 = gtk_radio_button_new_with_label(group, "Serial Port 3");
        gtk_widget_show(port2);
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(port2));
        port3 = gtk_radio_button_new_with_label(group, "Serial Port 4");
        gtk_widget_show(port3);
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(port3));
        other = gtk_radio_button_new_with_label(group, "Other");
        gtk_widget_show(other);

	ent_other = gtk_entry_new();
	gtk_widget_show(ent_other);

	if (strncmp(serial_port_prefix, serial_port, 9) == 0) {
		switch (serial_port[9]) {
			case '0':
				gtk_button_clicked(GTK_BUTTON(port0));
				break;
			case '1':
				gtk_button_clicked(GTK_BUTTON(port1));
				break;
			case '2':
				gtk_button_clicked(GTK_BUTTON(port2));
				break;
			case '3':
				gtk_button_clicked(GTK_BUTTON(port3));
				break;
			default:
				break;
		}
	} else {
 		gtk_button_clicked(GTK_BUTTON(other));
		gtk_entry_set_text(GTK_ENTRY(ent_other), serial_port);
	}

        gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), port0, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), port1, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), port2, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), port3, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), other, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), ent_other, FALSE, FALSE, 0);

	toggle = gtk_toggle_button_new();
	gtk_widget_show(toggle);
	gtk_widget_hide(toggle);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
                           toggle, FALSE, FALSE, 0);

        button = gtk_button_new_with_label("Save");
        gtk_widget_show(button);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
                           button, TRUE, TRUE, 0);

        cbutton = gtk_button_new_with_label("Cancel");
        gtk_widget_show(cbutton);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
                           cbutton, TRUE, TRUE, 0);

	if (wait_for_hide(dialog, button, cbutton) == 0)
		return;

	dlist = GTK_LIST(list)->selection;
	if (!dlist) {
		/* do nothing */
	} else {
		olist_item = GTK_OBJECT(dlist->data);
		sprintf(camera_model, "%s",
		(char*)gtk_object_get_data(olist_item,"model"));
		set_camera(camera_model);
	}

	if (GTK_WIDGET_STATE(port0) == GTK_STATE_ACTIVE) {
		sprintf(tempstring, "%s0", serial_port_prefix);
		strcpy(serial_port, tempstring);
	}
	if (GTK_WIDGET_STATE(port1) == GTK_STATE_ACTIVE) {
		sprintf(tempstring, "%s1", serial_port_prefix);
		strcpy(serial_port, tempstring);
	}
	if (GTK_WIDGET_STATE(port2) == GTK_STATE_ACTIVE) {
		sprintf(tempstring, "%s2", serial_port_prefix);
		strcpy(serial_port, tempstring);
	}
	if (GTK_WIDGET_STATE(port3) == GTK_STATE_ACTIVE) {
		sprintf(tempstring, "%s3", serial_port_prefix);
		strcpy(serial_port, tempstring);
	}
	if (GTK_WIDGET_STATE(other) == GTK_STATE_ACTIVE) {
		sprintf(tempstring, "%s",
			gtk_entry_get_text(GTK_ENTRY(ent_other)));
		strcpy(serial_port, tempstring);
	}
printf("serial port: %s\n", serial_port);
	save_config();
	gtk_widget_destroy(dialog);	
}

int  load_config() {

	char fname[1024];
	FILE *conf;

        sprintf(fname, "%s/gphotorc", gphotoDir);
        conf = fopen(fname, "r");
        if (!conf)
                return (0);
           else {
                fgets(fname, 100, conf);
                strncpy(serial_port, fname, strlen(fname)-1);
                fgets(fname, 100, conf);
                strncpy(camera_model, fname, strlen(fname)-1);
                fgets(fname, 100, conf);
                strncpy(post_process_script, fname, strlen(fname)-1);
                fclose(conf);
        }

	if (strcmp(camera_model, post_process_script) == 0)
		sprintf(post_process_script, "");

	return (1);
}

void save_config() {

	char gphotorc[1024];
	FILE *conf;


	sprintf(gphotorc, "%s/gphotorc", gphotoDir);
	conf = fopen(gphotorc, "w");
	fprintf(conf, "%s\n", serial_port);
	fprintf(conf, "%s\n", camera_model);
	fprintf(conf, "%s\n", post_process_script);

	fclose(conf);
}

void version_dialog() {

	error_dialog("
Version Information
-------------------

Current Version: v0.3

This developer's release should be much more stable than
the pre-release. A lot has changed internally, and we
are moving ahead and preparing to add support for many
different cameras.

There are more features (i.e. manipulation, batch save, ...)
and it should get the index a lot faster.

As always, report bugs gphoto-devel@lists.styx.net

Thanx much. :)
");

}
	
void usersmanual_dialog() {

	GtkWidget *dialog, *label, *scrwin, *button;

	dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog), "User's Manual");
	gtk_widget_set_usize(dialog, 460, 450);

	label = gtk_label_new("
gPhoto 0.3 User's Manual
------------------------

gPhoto was designed to be as intuitive as possible. The toolbar provides
shortcuts to many of the features listed below.


Getting Thumbnails/Previews
---------------------------
	In the \"Camera\" menu, select \"Get Index\"

Downloading Images
------------------
	After getting the index, select the images you want to download
	by clicking on them (border will turn red). Then, in the
	\"Camera\" menu, select \"Get Selected Images\".

Manipulating Images
-------------------
	After downloading an image, view it by clicking on its tab.
	Then, click the desired manipulation. Available are
	rotate clockwise, rotate counter-clockwise, flip horizontal,
	flip vertical, and resize.

	* For intensive image manipulation, fire up the GIMP!

Send to the GIMP
----------------
	Not currently supported. 

Saving Images
-------------
	Click on the tab of the image you want to save, and select
	\"Save\" from the \"File\" menu. Select the directory you want
	to save the image in, and type in a name for the image.
	The extension will determine which format the image is saved
	in.

	Example: image.jpg

	Supported: jpg gif png tif (and others...)
	(whatever is supported by your installation of imlib)

Batch Saving
------------

	[Outdated]

	Select \"Batch Save\" from the \"File\" menu. Then, select the
	directory that will store all the images and click \"OK\".

	* Currently, gPhoto only supports saving ALL images and/or
	  thumbnails.

	* Pictures are all stored as JPEG images by default.

Printing Images
---------------
	Click on the tab of the image you want to print, and select
	\"Print\" from the \"File\" menu.

	You can specify options to send to \"lpr\" in the supplied box.

	Note: it might take a minute for the image to print, depending
	on how fast the image is spooled.

Closing Downloaded Images
-------------------------
	Click on the tab of the image you want to close, and select
	\"Close\" from the \"File\" menu.

Deleting Images from the Camera
-------------------------------
	In the image index, select the images you wish to remove from
	the camera by clicking on the thumnail (border will turn red).
	Then, from the \"Camera\" menu, select \"Delete Selected Images\".
	A confirmation will appear. Click \"yes\" or \"no\".

Selecting the Serial Port
-------------------------
	Select \"Select Port\" from the \"Camera\" menu.

Configuring the Camera
----------------------
	Select \"Configure Camera\" from the \"Configure\" menu.

Live preview plugin
-------------------
        Select \"Live cam\" from the \"Plugins\" menu.
	Click on \"Take picture\" to update preview.

Command line mode
-----------------

	[Outdated. type \"gphoto -h\" for options]

	The command-line mode must be specified at run time
        with the \"-c\" parameter.
 
 	$ gphoto -c filename
 	where \"filename\" is the path and image name (+ extension)
 	to save a preview snapshot.
 
 	Example:
 	$ gphoto -c /home/httpd/html/livepic.jpg
 
 	The command line mode is handy, if you want to set up a web
 	camera, and use gphoto in a script, e.g. with Perl/PHP. :-)
 					  
Tips and Tricks
---------------
	* Make sure your camera is connected and turned ON. :-)
	* If something fails (getting index, configuring camera, ...)
	  try again. There are some things that need to be done still
	  to make sure the camera does what you want it to do.
 	* Have fun, and give us feedback if your camera fail or succeed
          to <gphoto-devel@lists.styx.net> and visit www.gphoto.org for
          regular updates!
");

	gtk_widget_show(label);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	scrwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrwin),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
 	gtk_widget_show(scrwin);
#ifndef GTK_HAVE_FEATURES_1_1_4
 	gtk_container_add(GTK_CONTAINER(scrwin), label);
#else
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrwin),
					      label);
#endif

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),scrwin);

        button = gtk_button_new_with_label("OK");
	gtk_widget_show(button);
        GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
        gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_destroy),
                            GTK_OBJECT(dialog));
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
                           button, TRUE, TRUE, 0);	
	gtk_widget_show(dialog);
	gtk_widget_grab_default(button);
}

void faq_dialog() {
   error_dialog("Please visit http://www.gphoto.org/help.php3 for the current FAQ list.");
}
 
void about_dialog() {
   
  error_dialog("
gPhoto version 0.3 Developer's Release

Copyright (C) 1998-99  Scott Fritzinger <scottf@scs.unr.edu>
                       Matt Martin <matt.martin@ieee.org>
                       Del Simmons <del@freespeech.com>
                       Bob Paauwe <bpaauwe@bobsplace.com>
                       Cliff Wright <cliff@snipe444.org>
                       Phill Hugo <phill@gphoto.org>
                       Beat Christen <spiff@longstreet.ch>
                       Brent D. Metz <bmetz@vt.edu>
                       Warren Baird <wjbaird@bigfoot.com>
                       Ole K. Aamot <oleaa@ifi.uio.no>

gPhoto uses the PhotoPC library (libeph_io).
Copyright (c) 1997,1998 Eugene G. Crosser <crosser@average.org>
Copyright (c) 1998 Bruce D. Lightner (DOS/Windows support)

Visit http://www.gphoto.org/ for updates.");
}
 
 void show_license() {
   
  error_dialog("
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

See http://www.gnu.org for more details on the GNU project."); 
}

/* used until callback is implemented */
void menu_selected (gchar *toPrint) {

	char Error[1024];
	
	sprintf(Error, "%s is not yet implemented", toPrint);
	error_dialog(Error);
}

void remove_thumbnail (int i) {

	int x=0;
	struct ImageInfo *node = &Thumbnails;
	struct ImageInfo *prev = node;

	while (x < i) {
		prev = node;
		node = node->next;
		x++;
	}

	prev->next = node->next;

	if (node->imlibimage!=NULL)
		gdk_imlib_kill_image(node->imlibimage);

	if (node->image!=NULL)
	  if (GTK_PIXMAP(node->image)->pixmap) {
	    gdk_pixmap_unref(GTK_PIXMAP(node->image)->pixmap);
	    gdk_imlib_free_pixmap(GTK_PIXMAP(node->image)->pixmap);	
	  }

	gtk_widget_destroy(node->button);
	free (node);
}

/* Place a thumbnail into the index at location corresponding to node */
void insert_thumbnail(struct ImageInfo *node) {
  char status[256], error[32];
  GdkPixmap *pixmap;
  GdkImlibImage *scaledImage;
  GtkWidget *vbox;
  /*  char *thumbname;*/
  int i=0;
  int w, h;
  struct ImageInfo *other=&Thumbnails;
  struct Image *im;

  /* maybe this info should be part of the imageinfo structure */
  while ((other!=node)&&(other!=NULL)){ // Find node in the list
    i++;
    other=other->next;
  };

  if (other==NULL) return;  // didn't find node in the list of thumbs

  sprintf(status, "Getting thumbnail %i...", i);
  update_status(status);

  /* Kill any previously loaded images, maybe this should just return*/
  if (node->image!=NULL)
    gtk_widget_destroy(node->image);
  if (node->imlibimage!=NULL)
    gdk_imlib_kill_image(node->imlibimage);


  if ((im = (*Camera->get_picture)(i, 1))==0) {
	sprintf(error, "Could not retrieve #%i", i);
	error_dialog(error);
	return;
  }
  node->imlibimage = gdk_imlib_load_image_mem(im->image, im->image_size);
  /*
  thumbname=find_tag(im,"ImageDescription");
  if (thumbname!=NULL) gtk_label_set_text  (GTK_LABEL(node->label)
					    ,thumbname);
  */
  free_image (im);

  w = node->imlibimage->rgb_width; 
  h = node->imlibimage->rgb_height;
  gdk_imlib_render(node->imlibimage, w, h);
  if ((w >= h) && (w > 80)) {
    h = h * 80 / w;
    w = 80;
  }
  if ((h >= w) && (h > 60)) {
    w = w * 60/ h;
    h = 60;
  }
  scaledImage = gdk_imlib_clone_scaled_image(node->imlibimage,w,h);
  gdk_imlib_kill_image(node->imlibimage);
  node->imlibimage = scaledImage;
  w = node->imlibimage->rgb_width;
  h = node->imlibimage->rgb_height;
  gdk_imlib_render(node->imlibimage, w, h);
  pixmap = gdk_imlib_move_image(node->imlibimage);
  node->image = gtk_pixmap_new(pixmap, NULL);  
  gtk_widget_show(node->image);

  /* this approach is a little dangerous...*/
  vbox=gtk_container_children(GTK_CONTAINER(node->button))->data;
  gtk_container_add(GTK_CONTAINER(vbox), 
		    node->image);

 };

/* intercept mouse click on a thumbnail button */
gint thumb_click( GtkWidget *widget,GdkEventButton *event,gpointer   callback_data ) {
        if (callback_data==NULL) return(FALSE);

	/* Double click will (re)-load the thumbnail*/
	if (event->type==GDK_2BUTTON_PRESS){
	  insert_thumbnail((struct ImageInfo *)callback_data);
	  return(TRUE);
	};

	return(FALSE);
 };

/* 
   get index of images and place in main page table 
        calling with getthumbs==0  makes a set of blank buttons
	                      !=0  downloads the thumbs for each
*/
void makeindex (int getthumbs) {

	int i;
	int num_pictures_taken;
	char status[256];

	GtkTooltips *tooltip;
	GtkStyle *style;
	GtkWidget *vbox;

	struct ImageInfo *node = &Thumbnails;

	update_status("Getting Index...");	

	while (Thumbnails.next != NULL)
		remove_thumbnail(1);

	if (index_table != NULL) {
		gtk_widget_destroy(index_table);
	}

	index_table = gtk_table_new(100,6,FALSE);
        gtk_widget_show(index_table);
#ifndef GTK_HAVE_FEATURES_1_1_4
        gtk_container_add(GTK_CONTAINER(index_window), index_table);
#else
        gtk_container_add(GTK_CONTAINER(index_vp), index_table);
#endif
	num_pictures_taken = (*Camera->number_of_pictures)();
fprintf(stderr, "num_pictures_taken is %d\n", num_pictures_taken);
	if (num_pictures_taken == -1) {
		error_dialog("Could not get the number of pictures");
		return;
	}

	for (i=1; i<=num_pictures_taken; i++) {
		node->next = malloc (sizeof(struct ImageInfo));
		node = node->next; node->next = NULL;

		sprintf(status, "Picture #%i", i);
		node->button = gtk_toggle_button_new();
		gtk_widget_show(node->button);
		tooltip = gtk_tooltips_new();
		gtk_tooltips_set_tip(tooltip,node->button,status, NULL);

		/* make a label for the thumbnail */
		sprintf(status, "%i", i);
		node->info=strdup(status);
		node->label=gtk_label_new(node->info);
		gtk_widget_show(node->label);

		gtk_widget_set_rc_style(node->button);
		style = gtk_style_copy(
		  gtk_widget_get_style(node->button));
		style->bg[GTK_STATE_ACTIVE].green=0;
		style->bg[GTK_STATE_ACTIVE].blue=0;
		gtk_widget_set_style(node->button, style);

		vbox = gtk_vbox_new(FALSE, 0);
		gtk_container_border_width(GTK_CONTAINER(vbox),3);
		gtk_widget_set_usize(vbox, 87, 78);

		gtk_widget_show(vbox);

		gtk_container_add(
			GTK_CONTAINER(node->button),vbox);
		gtk_container_add(GTK_CONTAINER(vbox),
				  node->label);

		/* Get the thumbnail */
		node->image=NULL;
		node->imlibimage=NULL;

		if (getthumbs) insert_thumbnail(node);

		gtk_signal_connect(GTK_OBJECT(node->button), 
				   "button_press_event",
				   GTK_SIGNAL_FUNC(thumb_click),
				   node);

		gtk_table_attach(GTK_TABLE(index_table),
				 node->button,
				 (i-1)%6,(i-1)%6+1,(i-1)/6,(i-1)/6+1,
				 GTK_FILL,GTK_FILL,5,5);
		update_progress((float)i/(float)num_pictures_taken);
	}
	update_progress(0);
	update_status("Done getting index.");	
}

/* get index of images and place in main page table */
void getindex () {
  makeindex(1);
}

/* get empty index of images and place in main page table */
void getindex_empty () {
  makeindex(0);
}

/* get selected pictures */
void getpics (char *pictype) {

	char status[256];
	int i=0;
	int x=0, y=0;	

	struct ImageInfo *node = &Thumbnails;
	
	while (node->next != NULL) {	
		node=node->next;
		if (GTK_TOGGLE_BUTTON(node->button)->active)
			x++;
	}
	if (x == 0) {
		update_status("Nothing selected for download.");
		return;
	}

	node = &Thumbnails;
	update_progress(0);
	while (node->next != NULL) {
		node = node->next; i++;
		if (GTK_TOGGLE_BUTTON(node->button)->active) {
			y++;
			if ((strcmp("i", pictype) == 0) ||
			    (strcmp("ti", pictype) == 0)) {
				sprintf(status, "Getting Image #%i...", i);
				update_status(status);
				appendpic(i, 0, TRUE, NULL);
			}
			if ((strcmp("t", pictype) == 0) ||
			    (strcmp("ti", pictype) == 0)) {			
				appendpic(i, 1, TRUE, NULL);
				sprintf(status, "Getting Thumbnail #%i...", i);
				update_status(status);
			}
			gtk_toggle_button_set_state(
				GTK_TOGGLE_BUTTON(node->button),
				FALSE);
			update_progress((float)y/(float)x);
		}
	}
	update_status("Done downloading.");
	update_progress(0);
}

void remove_image(int i) {

	int x=0;

	struct ImageInfo *node = &Images;
	struct ImageInfo *prev = node;


	while (x < i) {
		prev = node;
		node = node->next;
		x++;
	}

	prev->next = node->next;
	gdk_imlib_kill_image(node->imlibimage);
	gdk_pixmap_unref(GTK_PIXMAP(node->image)->pixmap);
	gdk_imlib_free_pixmap(GTK_PIXMAP(node->image)->pixmap);
	gtk_widget_destroy(node->image);
	gtk_widget_destroy(node->page);
/*	gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), i);*/
	free(node);
}

void closepic () {

	int currentPage;

	currentPage = gtk_notebook_current_page (GTK_NOTEBOOK(notebook));

	if (currentPage != 0) {
		remove_image(currentPage);
	}
}

void savepic (GtkWidget *widget, GtkFileSelection *fwin) {

	int i, x=0;
	char *fname;

	struct ImageInfo *node = &Images;

	gtk_widget_hide(GTK_WIDGET(fwin));
	i = gtk_notebook_current_page (GTK_NOTEBOOK(notebook));
	while (x < i) {
		node = node->next;
		x++;
	}

	if (i != 0) {
		fname = gtk_file_selection_get_filename(
				GTK_FILE_SELECTION(fwin));
		if (gdk_imlib_save_image(node->imlibimage, fname, NULL) == 0) {
			error_dialog(
			"Could not save image. Please make
			sure that you typed the image
			extension and that permissions for
			the directory are correct.");
			return;
		}
		update_status("Image Saved.");
	}
}

void openpic (GtkWidget *widget, GtkFileSelection *fwin) {

	char *fname, dir[1024];

	gtk_widget_hide(GTK_WIDGET(fwin));
	fname = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fwin));
	appendpic(0, 0, FALSE, fname);
}
	

/* save picture being viewed as jpeg */
void filedialog (gchar *a) {

	int currentPic;
	GtkWidget *filew;

	switch (a[0]) {
	   	case 's':
			filew = gtk_file_selection_new ("Save Image...");
			gtk_file_selection_set_filename(GTK_FILE_SELECTION
				(filew), filesel_cwd);
			gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION
			    (filew)->ok_button), "clicked",
			    (GtkSignalFunc) savepic,
			    filew);
			currentPic = gtk_notebook_current_page
					(GTK_NOTEBOOK(notebook));
			if (currentPic != 0)
	        		gtk_widget_show(filew);
			   else
				error_dialog("Saving the index is not yet supported.");
			break;

		case 'o':
			filew = gtk_file_selection_new ("Open Image...");
			gtk_file_selection_set_filename(GTK_FILE_SELECTION
				(filew), filesel_cwd);
			gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION
			    (filew)->ok_button), "clicked",
			    (GtkSignalFunc) openpic,
			    filew);
			gtk_widget_show(filew);
			break;
		default :
			break;
	}		
        gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
			    (filew)->cancel_button), "clicked",
			    (GtkSignalFunc) gtk_widget_hide,
                            GTK_OBJECT (filew));
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(filew),"*.*");
}

void print_pic () {

	int currentPic, pid, x=0;
	char command[1024], fname[1024];
	struct ImageInfo *node = &Images;

	GtkWidget *dialog, *label, *entry, *hbox, *okbutton, *cancelbutton;

	currentPic = gtk_notebook_current_page (GTK_NOTEBOOK(notebook));
	if (currentPic == 0) {
		error_dialog("Can't print the index yet.");
		return;
	}

        dialog = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(dialog), "Print Image...");
                
        hbox = gtk_hbox_new(FALSE, 5);
        gtk_widget_show(hbox);
        gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),
			hbox);

        label = gtk_label_new("Print Command:");
        gtk_widget_show(label);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
        gtk_box_pack_start_defaults(GTK_BOX(hbox), label);
                
        entry = gtk_entry_new();
        gtk_widget_show(entry);
        gtk_entry_set_text(GTK_ENTRY(entry), "lpr -r");
        gtk_box_pack_end_defaults(GTK_BOX(hbox),entry); 

	label = gtk_label_new(
"* The filename is appended to the end
   of the command.
* The \"-r\" flag is needed to delete 
  the temporary file. If not used, a 
  temporary file will be in your $HOME/.gphoto
  directory.");
        gtk_widget_show(label);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
        gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),
				    label);

        okbutton = gtk_button_new_with_label("Print");
        gtk_widget_show(okbutton);
        GTK_WIDGET_SET_FLAGS (okbutton, GTK_CAN_DEFAULT);
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->action_area),
                                    okbutton);

        cancelbutton = gtk_button_new_with_label("Cancel");
        gtk_widget_show(cancelbutton);
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->action_area),
                                    cancelbutton);

        gtk_widget_show(dialog);
        gtk_widget_grab_default(okbutton);

	if (wait_for_hide(dialog, okbutton, cancelbutton) == 0)
		return;

	while (x < currentPic) {
		node = node->next;
		x++;
	}

	pid = getpid();
	sprintf(fname, "%s/gphoto-%i-%i.jpg", gphotoDir, pid, currentPic);
	gdk_imlib_save_image(node->imlibimage, fname,NULL);

	update_status("Now spooling...");
	sprintf(command, "%s %s", gtk_entry_get_text(GTK_ENTRY(entry)),
		fname);

	system(command);
	update_status("Spooling done. Printing may take a minute.");

}

void select_all() {

	struct ImageInfo *node = &Thumbnails;

	while (node->next != NULL) {
		node = node->next;
		if (!GTK_TOGGLE_BUTTON(node->button)->active)
		gtk_button_clicked(GTK_BUTTON(node->button));
	}
}

void select_inverse() {

	struct ImageInfo *node = &Thumbnails;

	while (node->next != NULL) {
		node = node->next;
		gtk_button_clicked(GTK_BUTTON(node->button));
	}
}

void select_none() {

	struct ImageInfo *node = &Thumbnails;

	while (node->next != NULL) {
		node = node->next;
		if (GTK_TOGGLE_BUTTON(node->button)->active)
		  gtk_button_clicked(GTK_BUTTON(node->button));
	}
}

void color_dialog() {
	int i=0,currentPic;
	struct ImageInfo *node = &Images;

	currentPic = gtk_notebook_current_page (GTK_NOTEBOOK(notebook));
	if (currentPic == 0) {
	  update_status("Can't modify the index colors.");
	  return;
	}

	while (i < currentPic) {
                node = node->next;
                i++;
        }

	gtk_widget_show(GTK_WIDGET(img_edit_new(node)));
}

void resize_dialog() {

	int i=0, w, h, currentPic;
	char size[10];
	char *dimension;

	GtkWidget *dialog, *rbutton, *button, *toggle;
	GtkWidget *label, *hbox;
	GtkWidget *wentry, *hentry;

	GdkImlibImage *scaledImage;
	GdkPixmap *pixmap;

	struct ImageInfo *node = &Images;

	currentPic = gtk_notebook_current_page (GTK_NOTEBOOK(notebook));
	if (currentPic == 0) {
	  update_status("Can't scale the index.");
	  return;
	}

	while (i < currentPic) {
                node = node->next;
                i++;
        }

	dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog), "Resize Image...");

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),
                                    hbox);

	label = gtk_label_new("Width:");
	gtk_widget_show(label);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start_defaults(GTK_BOX(hbox), label);

	wentry = gtk_entry_new();
	gtk_widget_show(wentry);
	gtk_entry_set_max_length(GTK_ENTRY(wentry), 10);
	sprintf(size, "%i", node->imlibimage->rgb_width);
	gtk_entry_set_text(GTK_ENTRY(wentry), size);
	gtk_box_pack_end_defaults(GTK_BOX(hbox),wentry);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),
                                    hbox);
		
	label = gtk_label_new("Height:");
	gtk_widget_show(label);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start_defaults(GTK_BOX(hbox), label);

	hentry = gtk_entry_new();
	gtk_widget_show(hentry);
	sprintf(size, "%i", node->imlibimage->rgb_height);
	gtk_entry_set_text(GTK_ENTRY(hentry), size);
	gtk_entry_set_max_length(GTK_ENTRY(hentry), 10);
	gtk_box_pack_end_defaults(GTK_BOX(hbox), hentry);

	toggle = gtk_toggle_button_new();
	gtk_widget_show(toggle);
	gtk_widget_hide(toggle);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
                           toggle, TRUE, TRUE, 0);

	rbutton = gtk_button_new_with_label("Resize");
	gtk_widget_show(rbutton);
        GTK_WIDGET_SET_FLAGS (rbutton, GTK_CAN_DEFAULT);  
	gtk_signal_connect_object (GTK_OBJECT(rbutton), "clicked", 
			   GTK_SIGNAL_FUNC(ok_click),
			   GTK_OBJECT(dialog));
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->action_area),
					rbutton);

	button = gtk_button_new_with_label("Cancel");
	gtk_widget_show(button);
	gtk_signal_connect_object(GTK_OBJECT(button), "clicked", 
			   GTK_SIGNAL_FUNC(gtk_widget_hide),
			   GTK_OBJECT(dialog));
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->action_area),
					button);

	gtk_object_set_data(GTK_OBJECT(dialog), "button", "CANCEL");
	gtk_widget_show(dialog);
	gtk_widget_grab_default(rbutton);

	/* Wait for the user to close the window */
	if (wait_for_hide(dialog, rbutton, button) == 0)
		return;
	/* ------------------------------------- */

        dimension = gtk_entry_get_text(GTK_ENTRY(wentry));
        w = atoi(dimension);
        dimension = gtk_entry_get_text(GTK_ENTRY(hentry));
        h = atoi(dimension);

        scaledImage = gdk_imlib_clone_scaled_image(node->imlibimage,w,h);
        gdk_imlib_kill_image(node->imlibimage);

        node->imlibimage = scaledImage;
        w = node->imlibimage->rgb_width;
        h = node->imlibimage->rgb_height;
        gdk_imlib_render(node->imlibimage, w, h);
        pixmap = gdk_imlib_move_image(node->imlibimage);
        gtk_widget_hide(node->image);
        gtk_pixmap_set(GTK_PIXMAP(node->image), pixmap, NULL);
        gtk_widget_show(node->image);
	gtk_widget_destroy(dialog);
}

void manip_pic (gchar *Option) {

	int i=0, currentPic, w, h;
	struct ImageInfo *node = &Images;

	GdkPixmap *pixmap;
	
	currentPic = gtk_notebook_current_page (GTK_NOTEBOOK(notebook));
	if (currentPic == 0) {
	  update_status("Can't manipulate the index.");
/*  	  error_dialog("Can't manipulate the index."); */
	  return;
	}
	while (i < currentPic) {
		node = node->next;
		i++;
	}

	switch (Option[0]) {
		case 'r':
			gdk_imlib_rotate_image(node->imlibimage, 0);
			gdk_imlib_flip_image_horizontal(node->imlibimage);
			break;
		case 'l':
			gdk_imlib_flip_image_horizontal(node->imlibimage);
			gdk_imlib_rotate_image(node->imlibimage, 0);
			break;
		case 'h':
			gdk_imlib_flip_image_horizontal(node->imlibimage);
			break;
		case 'v':
			gdk_imlib_flip_image_vertical(node->imlibimage);
			break;
		default:
			break;
	}
        w = node->imlibimage->rgb_width;
        h = node->imlibimage->rgb_height;
        gdk_imlib_render(node->imlibimage, w, h);
        pixmap = gdk_imlib_move_image(node->imlibimage);
	gtk_widget_hide(node->image);
	gtk_pixmap_set(GTK_PIXMAP(node->image), pixmap, NULL);
	gtk_widget_show(node->image);
}

void summary_dialog() {
  message_window ( "Camera Summary", (*Camera->summary)(), GTK_JUSTIFY_FILL );
}

void scale (int factor) { /* Decreases image size by factor n % */

  int i=0, currentPic;
  int w, h;
  char w_size[10], h_size[10];
  GdkImlibImage *scaledImage;
  GdkPixmap *pixmap;
  
  struct ImageInfo *node = &Images;

  w = h = 0;

  currentPic = gtk_notebook_current_page (GTK_NOTEBOOK(notebook));
  if (currentPic == 0) {
    update_status("Can't scale the index!");
    return;
  }
  
  while (i < currentPic) {
    node = node->next;
    i++;
  }
  
  sprintf(w_size, "%i", node->imlibimage->rgb_width);
  sprintf(h_size, "%i", node->imlibimage->rgb_height);

  w = atoi(w_size);
  h = atoi(h_size);
  
  w = (w * factor)/100;
  h = (h * factor)/100;
  
  scaledImage = gdk_imlib_clone_scaled_image(node->imlibimage,w,h);
  gdk_imlib_kill_image(node->imlibimage);
  
  node->imlibimage = scaledImage;
  w = node->imlibimage->rgb_width;
  h = node->imlibimage->rgb_height;
  gdk_imlib_render(node->imlibimage, w, h);
  pixmap = gdk_imlib_move_image(node->imlibimage);
  gtk_widget_hide(node->image);
  gtk_pixmap_set(GTK_PIXMAP(node->image), pixmap, NULL);
  gtk_widget_show(node->image);
  update_status("Done.");
}
 
void scale_half () { /* Scales image size by 50% */
  update_status("Scaling image by 50%...");
  scale (50);
}

void scale_double () { /* Scales image size by 200% */
  update_status("Scaling image by 200%...");
  scale (200);
}

void save_images (gpointer data, guint action, GtkWidget *widget) {

	saveselectedtodisk(widget, "i");
}

void open_images (gpointer data, guint action, GtkWidget *widget) {

	getpics("i");
}

void save_thumbs (gpointer data, guint action, GtkWidget *widget) {

	saveselectedtodisk(widget, "t");
}

void open_thumbs (gpointer data, guint action, GtkWidget *widget) {

	getpics("t");
}

void save_both (gpointer data, guint action, GtkWidget *widget) {

	saveselectedtodisk(widget, "ti");
		/* don't prompt for directory when saving images */
}

void open_both (gpointer data, guint action, GtkWidget *widget) {

	getpics("ti");
}


void post_process_change (GtkWidget *widget, GtkWidget *win) {

	GtkWidget *dialog, *ok, *cancel, *label, *pp, *script;

	GdkPixmap *pixmap;
	GdkBitmap *bitmap;
	GtkStyle *style;


	dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog), "Post-Processing Options"); 

	ok = gtk_button_new_with_label("OK");
	gtk_widget_show(ok);
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		ok);

	cancel = gtk_button_new_with_label("Cancel");
	gtk_widget_show(cancel);
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		cancel);

	pp = gtk_check_button_new_with_label("Enable post-processing");
	gtk_widget_show(pp);
	if (post_process)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pp), TRUE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), pp,
		FALSE, FALSE, 0);

	label = gtk_label_new("Post-processing program:");
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label,
		TRUE, TRUE, 0);
	
	script = gtk_entry_new();
	gtk_widget_show(script);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_entry_set_text(GTK_ENTRY(script), post_process_script);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), script,
		FALSE, FALSE, 0);

	label = gtk_label_new(
"Note: gPhoto will replace \"%s\" in the script command-line
with the full path to the selected image. Please make sure the
script exists.
Example: /usr/local/bin/datestamp %s");
	gtk_widget_show(label);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label,
		TRUE, TRUE, 4);

	gtk_widget_show(dialog);

	/* Wait for them to close the dialog */
	if (wait_for_hide(dialog, ok, cancel) == 0)
		return;

	style = gtk_widget_get_style(win);

	post_process = (int)GTK_TOGGLE_BUTTON(pp)->active;
	sprintf(post_process_script, "%s",
		gtk_entry_get_text(GTK_ENTRY(script)));
	save_config();
	if (post_process)
		/* We are turning on post_process'ing */
		pixmap = gdk_pixmap_create_from_xpm_d(win->window, &bitmap,
                 	&style->bg[GTK_STATE_NORMAL],
			(gchar **)post_processing_on_xpm);
	   else
		/* We are turning off post_process'ing */
		pixmap = gdk_pixmap_create_from_xpm_d(win->window, &bitmap,
                	&style->bg[GTK_STATE_NORMAL],
			(gchar **)post_processing_off_xpm);
        gtk_pixmap_set(GTK_PIXMAP(post_process_pixmap), pixmap, bitmap);
}
