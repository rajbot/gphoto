#include "main.h"
#include "gphoto.h"
#include "callbacks.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>


/* Helper Functions ------------------------------------------
   ----------------------------------------------------------- */

void update_status(char *newStatus) {

	/*
		displays whatever is in string "newStatus" in the
		status bar at the bottom of the main window
	*/

	gtk_label_set(GTK_LABEL(status_bar), newStatus);
	while (gtk_events_pending())
		gtk_main_iteration();
}

void update_progress(float percentage) {

	/*
		sets the progress bar to percentage% at the bottom
		of the main window
	*/

	gtk_progress_bar_update(GTK_PROGRESS_BAR(progress), percentage);
	while (gtk_events_pending())
		gtk_main_iteration();
}

void error_dialog(char *Error) {

	/* 
	   Standard, run-of-the-mill message box
	*/

	GtkWidget *dialog, *label, *button;

	dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog), "gPhoto Message");
	gtk_container_border_width(GTK_CONTAINER(dialog), 5);
	label = gtk_label_new(Error);
	button = gtk_button_new_with_label("OK");
	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);     
	gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
			    GTK_SIGNAL_FUNC(gtk_widget_destroy),
                            GTK_OBJECT(dialog));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
			   label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
			   button, TRUE, TRUE, 0);
	gtk_widget_show(label);
	gtk_widget_show(button);
	gtk_widget_show(dialog);
	gtk_widget_grab_default (button);
}

void ok_click (GtkWidget *dialog) {

	gtk_object_set_data(GTK_OBJECT(dialog), "button", "OK");
	gtk_widget_hide(dialog);
}

int wait_for_hide (GtkWidget *dialog, GtkWidget *ok_button,
		   GtkWidget *cancel_button) {

	gtk_object_set_data(GTK_OBJECT(dialog), "button", "CANCEL");
	gtk_signal_connect_object(
			GTK_OBJECT(ok_button), "clicked",   
                        GTK_SIGNAL_FUNC(ok_click),
                        GTK_OBJECT(dialog));
        gtk_signal_connect_object(GTK_OBJECT(cancel_button),
                        "clicked",
                        GTK_SIGNAL_FUNC(gtk_widget_hide),
                        GTK_OBJECT(dialog));
        gtk_widget_show(dialog);
        while (GTK_WIDGET_VISIBLE(dialog))
                gtk_main_iteration();
        if (strcmp("CANCEL",
           (char*)gtk_object_get_data(GTK_OBJECT(dialog), "button"))==0)
                return 0;
	return 1;
}

void set_camera (char *model) {

	int i=0;

	while (strlen(cameras[i].name) > 0) {
		if (strcmp(model, cameras[i].name) == 0) {
			gtk_label_set(GTK_LABEL(library_name),
					cameras[i].name);
			Camera = cameras[i].library;
			if ((*Camera->initialize)() != 0)
				return;
		}
		i++;
	}
	error_dialog("Could not set camera library.");
}

void configure_call() {

	if ((*Camera->configure)() == 0)
		error_dialog(
"No configuration options.");
}

void takepicture_call() {

	char status[256];
	int picNum;

	update_status("Taking picture...");
	printf("Calling the take_picture function!");
	picNum = (*Camera->take_picture)();

	if (picNum == 0) {
		update_status("Could not take pictures.");
		return;
	}

	sprintf(status, "New picture is #%i", picNum);

	error_dialog(status);
	update_status("Done.");
}

void del_pics (GtkWidget *dialog) {

        int i=0;

        struct ImageInfo *node = &Thumbnails;

        gtk_widget_hide(dialog);
        update_status("Deleting selected pictures...");

        /* delete from camera... */
        while (node->next != NULL) {
                node = node->next; i++;
                if (GTK_TOGGLE_BUTTON(node->button)->active) {
                        if ((*Camera->delete_picture)(i) != 0) {
				remove_thumbnail(i);
        	                i--;
			}
                }
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


void appendpic (int picNum, int fromCamera, char *fileName) {

	int w, h;
	char fname[15], *openName;
	
	GtkWidget *scrwin, *label;
	GdkPixmap *pixmap;

	struct ImageInfo *node = &Images;

	while (node->next != NULL)
		node = node->next;

	node->next = malloc(sizeof(struct ImageInfo));
	node = node->next; node->next = NULL;

	if (fromCamera)
		node->imlibimage = (*Camera->get_picture)(picNum, 0);
	   else
		node->imlibimage = gdk_imlib_load_image(fileName);
	w = node->imlibimage->rgb_width;
        h = node->imlibimage->rgb_height;
        gdk_imlib_render(node->imlibimage, w, h);
        pixmap = gdk_imlib_move_image(node->imlibimage);
        node->image = gtk_pixmap_new(pixmap, NULL);
	gtk_widget_show(node->image);

	/* Append to notebook -------------------------------- */
	if (fromCamera) {
		sprintf(fname, "%i", picNum);
		node->info = strdup(fname);
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

#ifndef GTK_HAVE_FEATURES_1_1_3
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
	char gphotorc[1024];
	int i=0;

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
#ifndef GTK_HAVE_FEATURES_1_1_3
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

#ifdef linux
	if (strncmp("/dev/ttyS", serial_port, 9) == 0) {
#elif defined(__FreeBSD__) || defined(__NetBSD__)
	if (strncmp("/dev/tty0", serial_port, 9) == 0) {
#else
	if (strncmp("/dev/tty0", serial_port, 9) == 0) {
#endif
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
        GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
        gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                            GTK_SIGNAL_FUNC(ok_click),
                            GTK_OBJECT(dialog));
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
                           button, TRUE, TRUE, 0);
        cbutton = gtk_button_new_with_label("Cancel");
        gtk_widget_show(cbutton);
        GTK_WIDGET_SET_FLAGS (cbutton, GTK_CAN_DEFAULT);
        gtk_signal_connect_object(GTK_OBJECT(cbutton), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_hide),
                            GTK_OBJECT(dialog));
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
                           cbutton, TRUE, TRUE, 0);
	gtk_object_set_data(GTK_OBJECT(dialog), "button", "CANCEL");
        gtk_widget_show(dialog);
        gtk_widget_grab_default(button);

	/* Wait for the window to be closed */
        while (GTK_WIDGET_VISIBLE(dialog))
                gtk_main_iteration();
	/* -------------------------------- */

	if (strcmp("CANCEL",
           (char*)gtk_object_get_data(GTK_OBJECT(dialog), "button"))==0)
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
	gtk_widget_destroy(list);

	sprintf(gphotorc, "%s/gphotorc", gphotoDir);

	conf = fopen(gphotorc, "w");
	if (GTK_WIDGET_STATE(port0) == GTK_STATE_ACTIVE) {
#ifdef linux
		fprintf(conf, "/dev/ttyS0\n");
		sprintf(serial_port, "/dev/ttyS0");
#elif defined(__FreeBSD__) || defined(__NetBSD__)
		fprintf(conf, "/dev/tty00\n");
		sprintf(serial_port, "/dev/tty00");
#endif

	}
	if (GTK_WIDGET_STATE(port1) == GTK_STATE_ACTIVE) {
#ifdef linux
		fprintf(conf, "/dev/ttyS1\n");
		sprintf(serial_port, "/dev/ttyS1");
#elif defined(__FreeBSD__) || defined(__NetBSD__)
		fprintf(conf, "/dev/tty01\n");
		sprintf(serial_port, "/dev/tty01");
#endif
	}
	if (GTK_WIDGET_STATE(port2) == GTK_STATE_ACTIVE) {
#ifdef linux
		fprintf(conf, "/dev/ttyS2\n");
		sprintf(serial_port, "/dev/ttyS2");
#elif defined(__FreeBSD__) || defined(__NetBSD__)
		fprintf(conf, "/dev/tty02\n");
		sprintf(serial_port, "/dev/tty02");
#endif
	}
	if (GTK_WIDGET_STATE(port3) == GTK_STATE_ACTIVE) {
#ifdef linux
		fprintf(conf, "/dev/ttyS3\n");
		sprintf(serial_port, "/dev/ttyS3");
#elif defined(__FreeBSD__) || defined(__NetBSD__)
		fprintf(conf, "/dev/tty03\n");
		sprintf(serial_port, "/dev/tty03");
#endif
	}
	if (GTK_WIDGET_STATE(other) == GTK_STATE_ACTIVE) {
		fprintf(conf, "%s\n",
			gtk_entry_get_text(GTK_ENTRY(ent_other)));
		sprintf(serial_port, "%s",
			gtk_entry_get_text(GTK_ENTRY(ent_other)));
	}
	fprintf(conf, "%s\n", camera_model);
	fclose(conf);
	gtk_widget_destroy(dialog);
}

void version_dialog() {

	error_dialog("
Version Information
-------------------

Current Version: 0.2

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
gPhoto 0.2 User's Manual
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

	Select \"Batch Save\" from the \"File\" menu. Then, select the
	directory that will store all the images and click \"OK\".

	* Currently, gPhoto only supports saving ALL images and/or
	  thumbnails.

	* Pictures are all stored as JPEG images by default.

Printing Images
---------------
	Click on the tab of the image you want to print, and select
	\"Print\" from the \"File\" menu.
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
	Select \"Configure\" from the \"Camera\" menu.

Live preview plugin
-------------------
        Select \"Live cam\" from the \"Plugins\" menu.
	Click on \"Take picture\" to update preview.

Command line mode
-----------------
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
          to <gphoto-devel@lists.styx.net>.  Thanks for using gPhoto!
");
	gtk_widget_show(label);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	scrwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrwin),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
 	gtk_widget_show(scrwin);
#ifndef GTK_HAVE_FEATURES_1_1_3
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

error_dialog("
Please visit http://gphoto.fix.no/gphoto/faq.shtml
for the current FAQ list.

For information on using gPhoto, see the \"User's Manual\"
in the \"Help\" menu.");
}

void about_dialog() {

error_dialog("
GNU Photo version 0.2 Developer's Release

Copyright (C) 1998 Scott Fritzinger <scottf@scs.unr.edu>
Copyright (C) 1998 Ole Kristian Aamot <oleaa@ifi.uio.no>
GNU Photo is the ultimate free graphical application for digital
still cameras, based on GTK+.  It aims to be GNOME compliant.

GNU Photo uses the PhotoPC library (libeph_io).
Copyright (c) 1997,1998 Eugene G. Crosser <crosser@average.org>
Copyright (c) 1998 Bruce D. Lightner (DOS/Windows support)

You may distribute and/or use libeph_io for any purpose modified
or unmodified copies of the library, if you preserve the copyright
notice above.

GNU Photo is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for further details.
(http://www.gnu.org/copyleft/gpl.html) 

Visit http://gphoto.fix.no/gphoto/ for GNU Photo updates.");
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
void insert_thumbnail(struct ImageInfo *node){
  char status[256];
  GdkPixmap *pixmap;
  GtkWidget *vbox;
  int i=0;
  int w, h;
  struct ImageInfo *other=&Thumbnails;

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

  node->imlibimage = (*Camera->get_picture)(i, 1);
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
gint thumb_click( GtkWidget *widget,GdkEventButton *event,gpointer   callback_data ){
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
#ifndef GTK_HAVE_FEATURES_1_1_3
        gtk_container_add(GTK_CONTAINER(index_window), index_table);
#else
        gtk_container_add(GTK_CONTAINER(index_vp), index_table);
#endif
	num_pictures_taken = (*Camera->number_of_pictures)();
	fprintf(stderr, "num_pictures_taken is %d\n", num_pictures_taken);
	if (num_pictures_taken == -1)
		return;

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
void getpics () {

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
			sprintf(status, "Getting image %i (this might take a minute)", i);
			update_status(status);
			gtk_toggle_button_set_state(
				GTK_TOGGLE_BUTTON(node->button),
				FALSE);
			appendpic(i, TRUE, NULL);
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
/*	gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), i); */
	gtk_widget_destroy(node->page);
	free(node);
}

void closepic () {

	int currentPage;

	currentPage = gtk_notebook_current_page (GTK_NOTEBOOK(notebook));

	if (currentPage != 0) {
		remove_image(currentPage);
	}
}

void batch_save_dialog() {

	int i=1;
	int num_pictures_taken;
	char fname[1024], status[20];
	char *filesel_prefix, *filesel_dir, prefix[1024];
	struct tm *tmTime;
	time_t aTime;
	GdkImlibImage *toSave;
	GtkWidget *filesel, *label, *rad_im, *rad_thumb, *rad_both, *hbox;
	GtkWidget *entry;
	GSList *group;

	filesel = gtk_file_selection_new(
			"Select a directory to store the images...");

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

        rad_im = gtk_radio_button_new_with_label(NULL, "Pictures");
        gtk_widget_show(rad_im);
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(rad_im));
        rad_thumb = gtk_radio_button_new_with_label(group, "Thumbnails");
        gtk_widget_show(rad_thumb);
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(rad_thumb));
        rad_both = gtk_radio_button_new_with_label(group, "Both");
        gtk_widget_show(rad_both);
        gtk_button_clicked(GTK_BUTTON(rad_both));

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_box_pack_start_defaults(GTK_BOX(hbox), rad_im);
	gtk_box_pack_start_defaults(GTK_BOX(hbox), rad_thumb);
	gtk_box_pack_start_defaults(GTK_BOX(hbox), rad_both);
	gtk_box_pack_start_defaults(GTK_BOX(
		GTK_FILE_SELECTION(filesel)->main_vbox), hbox);
	gtk_box_reorder_child(GTK_BOX(
		GTK_FILE_SELECTION(filesel)->main_vbox), hbox, 5);

        gtk_signal_connect_object(GTK_OBJECT(
		GTK_FILE_SELECTION(filesel)->cancel_button), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_hide),
		GTK_OBJECT(filesel));

        gtk_signal_connect_object(GTK_OBJECT(
		GTK_FILE_SELECTION(filesel)->ok_button), "clicked",
		GTK_SIGNAL_FUNC(ok_click),
		GTK_OBJECT(filesel));

	gtk_object_set_data(GTK_OBJECT(filesel), "button", "CANCEL");
	gtk_widget_show(filesel);
	
	/* Wait for the window to be closed */
        while (GTK_WIDGET_VISIBLE(filesel))
                gtk_main_iteration();
        /* -------------------------------- */
 
	if (strcmp("CANCEL", 
	   (char*)gtk_object_get_data(GTK_OBJECT(filesel), "button"))==0)
		return;

	gtk_widget_hide(GTK_WIDGET(filesel));
	update_status("Saving all images...");

	filesel_dir = gtk_file_selection_get_filename(
			GTK_FILE_SELECTION(filesel));
	filesel_prefix = gtk_entry_get_text(GTK_ENTRY(entry));

	if (strlen(filesel_prefix) == 0)
		sprintf(prefix, "%sImage", filesel_dir);
	   else
		sprintf(prefix, "%s%s", filesel_dir, filesel_prefix);
	aTime = time(&aTime);
	tmTime = localtime(&aTime);
	num_pictures_taken = (*Camera->number_of_pictures)();
	if (num_pictures_taken == 0)
		return;

	update_progress(0);
	while (i <= num_pictures_taken) {
		sprintf(status, "Saving image %i", i);
		update_status(status);
		if ((GTK_WIDGET_STATE(rad_im) == GTK_STATE_ACTIVE) ||
		    (GTK_WIDGET_STATE(rad_both) == GTK_STATE_ACTIVE)) {
			toSave = (*Camera->get_picture)(i, 0);
			sprintf(fname, "%s-%i%02i%02i-%03i.jpg",
				prefix, tmTime->tm_year+1900,
				tmTime->tm_mon+1, tmTime->tm_mday, i);
			if (gdk_imlib_save_image(toSave, fname, NULL) == 0) {
				error_dialog("Could not save images.");
				return;
			}
			gdk_imlib_kill_image(toSave);
		}
		if ((GTK_WIDGET_STATE(rad_thumb) == GTK_STATE_ACTIVE) ||
		    (GTK_WIDGET_STATE(rad_both) == GTK_STATE_ACTIVE)) {
			toSave = (*Camera->get_picture)(i, 1);
			sprintf(fname, "%s-%i%02i%02i-%03i-thumb.jpg",
				prefix, tmTime->tm_year+1900,
				tmTime->tm_mon+1, tmTime->tm_mday, i);
			if (gdk_imlib_save_image(toSave, fname, NULL) == 0) {
				error_dialog("Could not save images.");
				return;
			}
			gdk_imlib_kill_image(toSave);
		}
		update_progress((float)i/(float)num_pictures_taken);
		i++;
	}
	sprintf(fname, "Done. Images saved as %s*.jpg", 
		gtk_file_selection_get_filename(GTK_FILE_SELECTION(filesel)));
	update_status(fname);
	update_progress(0);
	gtk_widget_destroy(GTK_WIDGET(filesel));
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

	char *fname;

	gtk_widget_hide(GTK_WIDGET(fwin));
	fname = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fwin));
	appendpic(0, FALSE, fname);
}
	

/* save picture being viewed as jpeg */
void filedialog (gchar *a) {

	int currentPic;
	GtkWidget *filew;

	switch (a[0]) {
	   	case 's':
			filew = gtk_file_selection_new ("Save Image...");
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

void send_to_gimp () {

	error_dialog("Send to GIMP not yet implemented.");
/*
	------------------------------------------------------
	Problem keeping this function from being used:
		need to delete a temporary image AFTER
		the GIMP is done loading it. don't know
		how to either force gimp to create a lock-file
		or wait and know when gimp is done loading.

	thought about just creating a temp directory in
	$HOME/.gphoto and then whenever gPhoto is started,
	it cleans out that directory... but that's bad, and
	should never have been thought up..

	thought about some sort of lockfile, but how would
	gimp know to create/remove it? ...

	just need to know when the gimp is done loading
	the image, then remove the temp file... 

	so we need input, if you know how... :) thanx..
			-SF
	------------------------------------------------------
	char **args;
	char fname[1024], rm[1024];
	int pid, currentPic, i=0;

	struct ImageInfo *node = &Images;
	currentPic = gtk_notebook_current_page (GTK_NOTEBOOK(notebook));
	if (currentPic == 0) {
		error_dialog("Can't send the index to the GIMP.");
		return;
	}

	while (i < currentPic) {
		node = node->next;
		i++;
	}

	sprintf(fname, "%s/gphoto-%i.jpg", gphotoDir, picCounter);
	sprintf(rm, "rm %s", fname);
	picCounter++;
	gdk_imlib_save_image(node->imlibimage, fname, NULL);

	pid = fork();
	if (pid == 0) {
		args = malloc(3*sizeof(char*));
		args[0] = malloc(5*sizeof(char));
		strcpy (args[0], "gimp");
		args[1] = malloc(20*sizeof(char));
		strcpy (args[1], fname);
		args[2] = NULL;
		execvp(args[0], args);
		_exit(0);
	}
	system(rm); 
*/
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
		error_dialog("Can't manipulate the index.");
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
	while (GTK_WIDGET_VISIBLE(dialog))
		gtk_main_iteration();

	if (strcmp("CANCEL",
           (char*)gtk_object_get_data(GTK_OBJECT(dialog), "button"))==0)
                return;

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
		error_dialog("Can't manipulate the index.");
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

	error_dialog((*Camera->summary)());
}
