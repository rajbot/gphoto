#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gphoto2.h>

#include "callbacks.h"
#include "gtkiconlist.h"
#include "interface.h"
#include "support.h"
#include "globals.h"
#include "util.h"

#include "../pixmaps/no_thumbnail.xpm"

void debug_print (char *message) {
	if (gp_gtk_debug)
		printf("%s\n", message);
}

gboolean toggle_icon (GtkWidget *widget, gpointer data) {

	GtkWidget *icon_list = (GtkWidget*) lookup_widget(gp_gtk_main_window, "icons");
	GtkIconListItem *item;
	GList *child = GTK_ICON_LIST(icon_list)->icons;

	while (child) {
		item = (GtkIconListItem*)child->data;
		if ((item->eventbox == widget)||(item->entry == widget)) {

		   if (item->state == GTK_STATE_SELECTED)
			gtk_icon_list_unselect_icon(GTK_ICON_LIST(icon_list), item);
		     else
			gtk_icon_list_select_icon(GTK_ICON_LIST(icon_list), item);
		   idle();
		   gtk_entry_set_position(GTK_ENTRY(item->entry), 10000);
		   return TRUE;
		}
		child = child->next;
	}
	return TRUE;
}

void idle() {
	/* Let GTK do some processing */

        while (gtk_events_pending())
                gtk_main_iteration();
	usleep(100000);
        while (gtk_events_pending())
                gtk_main_iteration();
}


void icon_resize(GtkWidget *window) {

	GtkWidget *icon_list = (GtkWidget*)lookup_widget(gp_gtk_main_window, "icons");
	int x, y;

	gdk_window_get_size(gp_gtk_main_window->window, &x, &y);

	if (abs(gp_gtk_old_width-x)>60) {
		gtk_icon_list_freeze(GTK_ICON_LIST(icon_list));
		gtk_icon_list_thaw(GTK_ICON_LIST(icon_list));
		gp_gtk_old_width = x;
	}

}

int camera_set() {

	GtkWidget *camera_tree, *message, *message_label, *camera_label, *icon_list;
	CameraPortSettings ps;
	char camera[1024], port[1024], speed[1024];

	/* Mark camera as not init'd */
	gp_gtk_camera_init = 0;

	/* Create a transient window for "Initizlizing camera" message */
	message = create_message_window_transient();
	message_label = (GtkWidget*) lookup_widget(message, "message");
	gtk_label_set_text(GTK_LABEL(message_label), _("Initializing camera..."));

	/* Retrieve which camera to use */
	if (gp_setting_get("camera", camera)==GP_ERROR) {
		gp_message(_("You must choose a camera"));
		camera_select();
	}

	/* Retrieve the port to use */
	gp_setting_get("port", port);

	/* Retrieve the speed to use */
	gp_setting_get("speed", speed);

	/* Set up the camera initialization */
	strcpy(ps.path, port);
	if (strlen(speed)>0)
		ps.speed = atoi(speed);
	   else
		ps.speed = 0; /* use the default speed */
	gtk_widget_show(message);
	idle();

	/* Set the camera model in the gPhoto library */
	if (gp_camera_set_by_name(camera, &ps)==GP_ERROR) {
		gtk_widget_destroy(message);
		return (GP_ERROR);
	}

	/* Set the current folder */
	if (gp_folder_set("/")==GP_ERROR) {
		gtk_widget_destroy(message);
		return (GP_ERROR);
	}

	/* Set the current camera model */
	camera_label = (GtkWidget*) lookup_widget(gp_gtk_main_window, "camera_label");
	gtk_label_set_text(GTK_LABEL(camera_label), camera);
	gtk_widget_destroy(message);

	/* Clear out the icon listing */
	icon_list = (GtkWidget*) lookup_widget(gp_gtk_main_window, "icons");
	gtk_icon_list_clear (GTK_ICON_LIST(icon_list));

	/* Clear out the folder tree */
	camera_tree = (GtkWidget*) lookup_widget(gp_gtk_main_window, "camera_tree");
	gtk_object_remove_data(GTK_OBJECT(camera_tree), "expanded");
	gtk_tree_item_collapse(GTK_TREE_ITEM(camera_tree));

	/* Mark the camera as init'd */
	gp_gtk_camera_init = 1;

	return (GP_OK);
}

int main_quit(GtkWidget *widget, gpointer data) {

	char buf[1024];
	int x, y;

	/* Save the window size */
	gdk_window_get_size(gp_gtk_main_window->window, &x, &y);
	sprintf(buf, "%i", x);
	gp_setting_set("width", buf);
	sprintf(buf, "%i", y);
	gp_setting_set("height", buf);

	if (gp_gtk_camera_init)
		gp_exit();
	gtk_main_quit();

	return (FALSE);
}

/* File operations */
/* ----------------------------------------------------------- */

void append_photo(CameraFile *file) {
	debug_print("append photo");

}

void open_photo() {
	
	GtkWidget *filesel;
	char buf[1024];

	debug_print("open photo");

	filesel = gtk_file_selection_new("Open a photo");
	gp_setting_get("cwd", buf);
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(filesel), buf);

	if (wait_for_hide(filesel, GTK_FILE_SELECTION(filesel)->ok_button,
	    GTK_FILE_SELECTION(filesel)->cancel_button)==0)
		return;
}

void open_directory() {
	debug_print("open directory");
}

void toggle_sensitivity (GtkWidget *button, gpointer data) {

	GtkWidget *widget = (GtkWidget*)data;

	if (GTK_WIDGET_SENSITIVE(widget))
		gtk_widget_set_sensitive(widget, FALSE);
	   else
		gtk_widget_set_sensitive(widget, TRUE);
}

void save_selected_photos() {

	GtkWidget *icon_list, *window, *ok, *cancel, 
		  *use_camera_filename, *save_photos, *save_thumbs, *prefix_entry;
	GtkIconListItem *item;
	CameraFile *f;
	char msg[1024], fname[1024];
	char *path, *prefix=NULL, *slash;
	int num=0, x;

	debug_print("save selected photo");

	/* Count the number of icons selected */
	icon_list = (GtkWidget*)lookup_widget(gp_gtk_main_window, "icons");
	for (x=0; x<GTK_ICON_LIST(icon_list)->num_icons; x++) {
		item = gtk_icon_list_get_nth(GTK_ICON_LIST(icon_list), x);
		if (item->state == GTK_STATE_SELECTED)
			num++;
	}

	if (num == 0) {
		gp_message("Please select some photos to save first");
		return;
	}

	if (num > 1)
		window = create_save_window(1);
	   else
		window = create_save_window(0);

	ok = GTK_FILE_SELECTION(window)->ok_button;
	cancel = GTK_FILE_SELECTION(window)->cancel_button;

	if (wait_for_hide(window, ok, cancel)==0)
		return;

	/* Get some settings from the file selection window */
	path = gtk_file_selection_get_filename(GTK_FILE_SELECTION(window));	
	use_camera_filename = (GtkWidget*)lookup_widget(window, "use_camera_filename");
	save_photos = (GtkWidget*)lookup_widget(window, "save_photos");
	save_thumbs = (GtkWidget*)lookup_widget(window, "save_thumbs");

	if (num > 1) {
		prefix_entry = (GtkWidget*)lookup_widget(window, "prefix");
		prefix = gtk_entry_get_text(GTK_ENTRY(prefix_entry));
	}

	gp_progress(0.00);
	gtk_widget_show(gp_gtk_progress_window);
	for (x=0; x<GTK_ICON_LIST(icon_list)->num_icons; x++) {
		item = gtk_icon_list_get_nth(GTK_ICON_LIST(icon_list), x);
		if (item->state == GTK_STATE_SELECTED) {
		   /* Save the photo */
		   if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(save_photos))) {
			sprintf(msg, "Saving photo #%04i of %04i", x+1, num);
			gp_message(msg);
			f = gp_file_new();
			gp_file_get(x, f);
			/* determine the name to use */
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_camera_filename))) {
			   if (strlen(f->name)>0) {
				/* If the camera provided a filename */
				slash = strrchr(path, '/');
				*slash = 0;
				sprintf(fname, "%s/%s", path, f->name);
				*slash = '/';
			   } else {
				/* If not, error, and return */
				gp_message(_("Camera did not suggest a filename.\nPlease select \"Save\" again and specify a filename yourself."));
				gp_file_free(f);
				return;
			   }
			} else {
			   if (num == 1) {
				strcpy(fname, path);
			   } else {
				slash = strrchr(f->type, '/');
				slash++;
				if (prefix) {
					if (slash)
						sprintf(fname, "%s%s%04i.%s", path, prefix, x, slash);
					  else
						sprintf(fname, "%s%s%04i", path, prefix, x);
				} else {
					if (slash)
						sprintf(fname, "%sphoto%04i.%s", path, x, slash);
					   else
						sprintf(fname, "%sphoto%04i", path, x);
				}
			   }
			}
			/* check for existing file */
			if (file_exists(fname)) {
				sprintf(msg, "%s already exists. Overwrite?", fname);
				gtk_widget_hide(gp_gtk_progress_window);
				if (gp_confirm(msg))
					gp_file_save(f, fname);
				gtk_widget_show(gp_gtk_progress_window);
			} else {
				gp_file_save(f, fname);
			}

			gp_file_free(f);
		   }

		   /* Save the thumbnail */
		   if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(save_thumbs))) {
			sprintf(msg, "Saving thumbnail #%04i of %04i", x+1, num);
			gp_status(msg);
			f = gp_file_new();
			gp_file_get_preview(x, f);
			/* determine the name to use */
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_camera_filename))) {
			   if (strlen(f->name)>0) {
				/* If the camera provided a filename */
				slash = strrchr(path, '/');
				*slash = 0;
				sprintf(fname, "%s/thumb_%s", path, f->name);
				*slash = '/';
			   } else {
				/* If not, error, and return */
				gp_message(_("Camera did not suggest a filename.\nPlease select \"Save\" again and specify a filename yourself."));
				gp_file_free(f);
				return;
			   }
			} else {
			   if (num == 1) {
				strcpy(fname, path);
			   } else {
				if (prefix)
					sprintf(fname, "%sthumb_%s%04i", path, prefix, x);
				   else
					sprintf(fname, "%sthumb_%04i", path, x);
			   }
			}
			/* check for existing file */
			if (file_exists(fname)) {
				sprintf(msg, "%s already exists. Overwrite?", fname);
				if (gp_confirm(msg))
					gp_file_save(f, fname);
			} else {
				gp_file_save(f, fname);
			}
			gp_file_free(f);
		   }
		}
	}
	gtk_widget_destroy(window);
	gp_progress(0.00);
	gtk_widget_hide(gp_gtk_progress_window);
}

void export_gallery() {
	debug_print("export gallery");

}

void close_photo() {
	debug_print("close photo");

}

/* Selection operations */
/* ----------------------------------------------------------- */

void select_all() {
	GtkWidget *icon_list = (GtkWidget*) lookup_widget(gp_gtk_main_window, "icons");
	GtkIconListItem *item;
	int x;

	debug_print("select all");

	for (x=0; x<GTK_ICON_LIST(icon_list)->num_icons; x++) {
		item = gtk_icon_list_get_nth(GTK_ICON_LIST(icon_list), x);
		gtk_icon_list_select_icon(GTK_ICON_LIST(icon_list),item);
	}
}

void select_inverse() {
	GtkWidget *icon_list = (GtkWidget*) lookup_widget(gp_gtk_main_window, "icons");
	GtkIconListItem *item;
	int x;

	debug_print("select inverse");

	for (x=0; x<GTK_ICON_LIST(icon_list)->num_icons; x++) {
		item = gtk_icon_list_get_nth(GTK_ICON_LIST(icon_list), x);
		if (item->state == GTK_STATE_SELECTED)
			gtk_icon_list_unselect_icon(GTK_ICON_LIST(icon_list),item);
		   else
			gtk_icon_list_select_icon(GTK_ICON_LIST(icon_list),item);
	}
}

void select_none() {
	GtkWidget *icon_list = (GtkWidget*) lookup_widget(gp_gtk_main_window, "icons");
	GtkIconListItem *item;
	int x;

	debug_print("select none");

	for (x=0; x<GTK_ICON_LIST(icon_list)->num_icons; x++) {
		item = gtk_icon_list_get_nth(GTK_ICON_LIST(icon_list), x);
		gtk_icon_list_unselect_icon(GTK_ICON_LIST(icon_list),item);
	}
}

/* Folder Callbacks */
/* ----------------------------------------------------------- */

void folder_refresh (GtkWidget *widget, gpointer data) {

	GtkWidget *icon_list;

	debug_print("folder refresh");

	if (!gp_gtk_camera_init)
		if (camera_set()==GP_ERROR) {return;}

	icon_list = (GtkWidget*) lookup_widget(gp_gtk_main_window, "icons");
	gtk_icon_list_clear (GTK_ICON_LIST(icon_list));

	camera_index();
}

void folder_set (GtkWidget *tree_item, gpointer data) {

	char buf[1024];
	char *path = (char*)gtk_object_get_data(GTK_OBJECT(tree_item), "path");

	debug_print("folder set");

	if (!gp_gtk_camera_init)
		if (camera_set()==GP_ERROR) {return;}

	sprintf(buf, "camera folder path = %s", path);
	debug_print(buf);

	if (gp_folder_set(path)==GP_ERROR) {
		sprintf(buf, _("Could not open folder:\n%s"), path);
		gp_message(buf);
		return;

	}

	camera_index();
}

GtkWidget *folder_item (GtkWidget *tree, char *text) {
	/* Create an item in the "tree" with the label of text and a folder icon */

	return (tree_item_icon(tree, text, "folder.xpm"));
}

GtkWidget *tree_item_icon (GtkWidget *tree, char *text, char *icon_name) {
	/* Create an item in the "tree" with the label of text and an icon */

	GtkWidget *item, *hbox, *pixmap, *label, *subtree, *subitem;

	item = gtk_tree_item_new();
	gtk_widget_show(item);
	gtk_tree_append(GTK_TREE(tree), item);
	gtk_signal_connect(GTK_OBJECT(item), "select", 
		GTK_SIGNAL_FUNC(folder_set),NULL);
	gtk_signal_connect_after(GTK_OBJECT(item), "expand", 
		GTK_SIGNAL_FUNC(folder_expand),NULL);

	hbox = gtk_hbox_new(FALSE, 3);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(item), hbox);

	pixmap = create_pixmap(gp_gtk_main_window, icon_name);
	gtk_widget_show(pixmap);
	gtk_box_pack_start(GTK_BOX(hbox), pixmap, FALSE, FALSE, 0);

	label = gtk_label_new(text);
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	subtree = gtk_tree_new();
	gtk_widget_show(subtree);
	gtk_tree_item_set_subtree (GTK_TREE_ITEM(item), subtree);

	subitem = gtk_tree_item_new();
	gtk_widget_show(subitem);
	gtk_tree_append(GTK_TREE(subtree), subitem);
	gtk_object_set_data(GTK_OBJECT(subitem), "blech", (gpointer)"foo");
	
	return (item);
}

void folder_expand (GtkWidget *tree_item, gpointer data) {

	GtkWidget *tree, *item;
	CameraFolderInfo list[256];
	char *path = (char*)gtk_object_get_data(GTK_OBJECT(tree_item), "path");
	char buf[1024];
	int x, count=0;

	debug_print("folder_expand");

	if (!gp_gtk_camera_init)
		if (camera_set()==GP_ERROR) {return;}

	/* See if we've expanded this before! */
	if (gtk_object_get_data(GTK_OBJECT(tree_item), "expanded")) return;

	/* Count however many folders are in this folder */
	if ((count = gp_folder_list(path, list))==GP_ERROR) {
		sprintf(buf, _("Could not open folder\n%s"), path);
		gp_message(buf);
		return;
	}

	/* Remove any existing subtree */
	gtk_tree_item_remove_subtree(GTK_TREE_ITEM(tree_item));

	if (count == 0)
		return;

	/* Create a new subtree */
	tree = gtk_tree_new();
	gtk_widget_show(tree);
	gtk_tree_item_set_subtree(GTK_TREE_ITEM(tree_item), tree);

	/* Append the new folders to the new subtree */
	for (x=0; x<count; x++) {
		if (strcmp(path, "/")==0)
			sprintf(buf, "/%s", list[x].name);
		   else
			sprintf(buf, "%s/%s", path, list[x].name);
		item = folder_item(tree, list[x].name);
		gtk_object_set_data(GTK_OBJECT(item), "path", strdup(buf));
	}

	/* Mark this item as expanded (so we don't have to build the folder again */
	gtk_object_set_data(GTK_OBJECT(tree_item), "expanded", "yes");

	/* Now, actually expand the newly built tree */
	gtk_tree_item_expand(GTK_TREE_ITEM(tree_item));
}

/* Camera operations */
/* ----------------------------------------------------------- */

void camera_select_update_port(GtkWidget *entry, gpointer data) {

	GtkWidget *window, *speed;
	CameraPortType type = GP_PORT_NONE;
	char *t, *new_port, *path;
	char buf[1024];

	new_port = (char*)gtk_entry_get_text(GTK_ENTRY(entry));
	if (strlen(new_port)==0)
		return;

	sprintf(buf, "Selected port: %s", new_port);
	debug_print(buf);

	window = (GtkWidget*)data;
	speed  = (GtkWidget*)lookup_widget(window, "speed");
	t      = (char*)gtk_object_get_data (GTK_OBJECT(window), new_port);
	if (!t) {
		debug_print("port type not being set");
		return;
	}

	type   = (CameraPortType)atoi(t);
	sprintf(buf, "%s path", new_port);
	path = gtk_object_get_data(GTK_OBJECT(window), buf);

	/* Disabled by default */
	gtk_widget_set_sensitive(GTK_WIDGET(speed), FALSE);
	if (type == GP_PORT_SERIAL)
		gtk_widget_set_sensitive(GTK_WIDGET(speed), TRUE);
	   else
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(speed)->entry), "");

}

void camera_select_update_camera(GtkWidget *entry, gpointer data) {

	/* populate the speed list */
	GtkWidget *window, *port, *speed;
	GList *port_list, *speed_list;
	CameraAbilities a;
	CameraPortInfo info;
	char *new_camera;
	char buf[1024], buf1[1024];
	int num_ports, x=0, append;

	new_camera = (char*)gtk_entry_get_text(GTK_ENTRY(entry));
	if (strlen(new_camera)==0)
		return;

	window = (GtkWidget*)data;
	port   = (GtkWidget*)lookup_widget(window, "port");
	speed  = (GtkWidget*)lookup_widget(window, "speed");

	sprintf(buf, "Selected camera: %s", new_camera);
	debug_print(buf);

	if (gp_camera_abilities_by_name(new_camera, &a)==GP_ERROR) {
		sprintf(buf, _("Could not get abilities for %s"), new_camera);
		gp_message(buf);
		return;
	}

	/* Get the number of ports */
	if ((num_ports = gp_port_count())==GP_ERROR) {
		gp_message(_("Could not retrieve number of ports"));
		return;
	}

	/* Free the old port list?? */

	/* populate the port list */
	port_list = g_list_alloc();
	for (x=0; x<num_ports; x++) {
		append=0;
		if (gp_port_info(x, &info)==GP_OK) {
			if ((info.type == GP_PORT_SERIAL) && (a.serial))
				append=1;
			if ((info.type == GP_PORT_PARALLEL) && (a.parallel))
				append=1;
			if ((info.type == GP_PORT_IEEE1394) && (a.ieee1394))
				append=1;
			if ((info.type == GP_PORT_NETWORK) && (a.network))
				append=1;
			if ((info.type == GP_PORT_USB) && (a.usb))
				append=1;
			if (append) {
				sprintf(buf, "%s (%s)", info.name, info.path);
				port_list = g_list_append(port_list, strdup(buf));

				/* Associate a path with this entry */
				sprintf(buf1, "%s path", buf);
				gtk_object_set_data(GTK_OBJECT(window), 
					buf1, (gpointer)strdup(info.path));

				/* Associate a type with this entry */
				sprintf(buf1, "%i", info.type);
				gtk_object_set_data(GTK_OBJECT(window), 
					buf, (gpointer)strdup(buf1));
			}
		}  else {
			gp_message(_("Error retrieving the port list"));
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(port), port_list);

	/* Free the old speed list?? */

	/* Populate the speed list */
	speed_list = g_list_alloc();
	x=0;
	while (a.speed[x] != 0) {
		sprintf(buf, "%i", a.speed[x++]);
		speed_list = g_list_append(speed_list, strdup(buf));
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(speed), speed_list);

	gtk_widget_set_sensitive(GTK_WIDGET(port), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(speed), FALSE);
	if (a.serial || a.parallel || a.ieee1394 || a.network || a.usb)
		gtk_widget_set_sensitive(GTK_WIDGET(port), TRUE);
}

void camera_select() {
	GtkWidget *window, *ok, *cancel, *camera, *port, *speed;
	GList *camera_list;
	int num_cameras, x;
	char buf[1024];
	char *camera_name, *port_name, *port_path, *speed_name;
	
	debug_print("camera select");

	/* Get the number of cameras */
	if ((num_cameras = gp_camera_count())==GP_ERROR) {
		gp_message(_("Could not retrieve number of cameras"));
		return;
	}

	window = create_select_camera_window();
	ok     = (GtkWidget*)lookup_widget(window, "ok");
	cancel = (GtkWidget*)lookup_widget(window, "cancel");
	camera = (GtkWidget*)lookup_widget(window, "camera");
	port   = (GtkWidget*)lookup_widget(window, "port");
	speed  = (GtkWidget*)lookup_widget(window, "speed");

	gtk_widget_set_sensitive(GTK_WIDGET(port), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(speed), FALSE);

	/* populate the camera list */
	camera_list = g_list_alloc();
	for (x=0; x<num_cameras; x++) {
		if (gp_camera_name(x, buf)==GP_OK)
			camera_list = g_list_append(camera_list, strdup(buf));
		   else
			camera_list = g_list_append(camera_list, "ERROR");
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(camera), camera_list);

	/* Update the dialog if they change the camera */
	gtk_signal_connect(GTK_OBJECT(GTK_COMBO(camera)->entry), "changed",
		GTK_SIGNAL_FUNC (camera_select_update_camera), (gpointer)window);

	/* Update the dialog if they change the port */
	gtk_signal_connect(GTK_OBJECT(GTK_COMBO(port)->entry), "changed",
		GTK_SIGNAL_FUNC (camera_select_update_port), (gpointer)window);

	/* Retrieve the saved values */
	if (gp_setting_get("camera", buf)==GP_OK)
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(camera)->entry), buf);
	if (gp_setting_get("port name", buf)==GP_OK)
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(port)->entry), buf);
	if (gp_setting_get("speed", buf)==GP_OK)
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(speed)->entry), buf);

camera_select_again:
	if (wait_for_hide(window, ok, cancel) == 0)
		return;

	idle(); /* let GTK catch up */

	camera_name = (char*)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(camera)->entry));
	port_name   = (char*)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(port)->entry));
	speed_name  = (char*)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(speed)->entry));

	if (strlen(camera_name)==0) {
		gp_message(_("You must choose a camera model"));
		goto camera_select_again;
	}

	if ((strlen(port_name)==0)&&(GTK_WIDGET_SENSITIVE(port))) {
		gp_message(_("You must choose a port"));
		goto camera_select_again;
	}

	if ((strlen(speed_name)==0)&&(GTK_WIDGET_SENSITIVE(speed))) {
		gp_message(_("You must choose a port speed"));
		goto camera_select_again;
	}

	gp_setting_set("camera", camera_name);

	if (GTK_WIDGET_SENSITIVE(port)) {
		sprintf(buf, "%s path", port_name);
		port_path = (char*)gtk_object_get_data(GTK_OBJECT(window), buf);
		gp_setting_set("port name", port_name);
		gp_setting_set("port", port_path);
	}  else {
		gp_setting_set("port name", "");
		gp_setting_set("port", "");
	}

	if (GTK_WIDGET_SENSITIVE(speed))
		gp_setting_set("speed", speed_name);
	   else
		gp_setting_set("speed", "");


	if (camera_set()==GP_ERROR)
		goto camera_select_again;

	/* Clean up */
	gtk_widget_destroy(window);
}

void camera_index () {

	CameraFile *f;
	CameraAbilities a;
	GtkWidget *icon_list, *camera_tree, *use_thumbs;
	GtkIconListItem *item;
	GdkPixmap *pixmap;
	GdkBitmap *bitmap;
	char buf[1024];
	int x, count=0, get_thumbnails = 1;

	debug_print("camera index");
	
	if (!gp_gtk_camera_init)
		if (camera_set()==GP_ERROR) {return;}

	icon_list   = (GtkWidget*) lookup_widget(gp_gtk_main_window, "icons");
	GTK_ICON_LIST(icon_list)->is_editable = FALSE;

	if (gp_setting_get("camera", buf)==GP_ERROR) {
		gp_message(_("ERROR: please choose your camera model again"));
		camera_select();
		return;
	}
	if (gp_camera_abilities_by_name(buf, &a)==GP_ERROR) {
		gp_message(_("Could not retrieve the camera's abilities"));
		return;
	}

	if ((count = gp_file_count())==GP_ERROR) {
		gp_message(_("Could not retrieve the number of pictures"));
		return;
	}

	if (count == 0)
		return;

	gtk_icon_list_clear (GTK_ICON_LIST(icon_list));
	camera_tree = (GtkWidget*) lookup_widget(gp_gtk_main_window, "folder_tree");
	use_thumbs  = (GtkWidget*) lookup_widget(gp_gtk_main_window, "use_thumbs");
	x=0;
	gtk_widget_set_sensitive(camera_tree, FALSE);
	gp_progress(0.00);
	gtk_widget_show(gp_gtk_progress_window);
	while ((x<count)&&(GTK_WIDGET_VISIBLE(gp_gtk_progress_window))) {
		sprintf(buf,_("Getting Thumbnail #%04i of %04i"), x+1, count);
		gp_message(buf);
		idle();
		if ((get_thumbnails)&&
		    (a.file_preview)&&
		    (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_thumbs)))) {
			/* Get the thumbnails */
			f = gp_file_new();
			sprintf(buf, "Photo #%04i", x);
			if (gp_file_get_preview(x, f) == GP_OK) {
				gdk_image_new_from_data(f->data,f->size,1,&pixmap,&bitmap);
				item = gtk_icon_list_add_from_data(GTK_ICON_LIST(icon_list),
					no_thumbnail_xpm,buf,NULL);
				gtk_pixmap_set(GTK_PIXMAP(item->pixmap), pixmap, bitmap);
			} else {
				item = gtk_icon_list_add_from_data(GTK_ICON_LIST(icon_list),
					no_thumbnail_xpm,buf,NULL);
			}
			gp_file_free(f);
		} else {
			/* Use a dummy icon */
			sprintf(buf, "Photo #%04i", x);
			item = gtk_icon_list_add_from_data(GTK_ICON_LIST(icon_list),
				no_thumbnail_xpm,buf,NULL);
		}
		gtk_signal_connect(GTK_OBJECT(item->eventbox), "button_press_event",
			GTK_SIGNAL_FUNC(toggle_icon), NULL);
		gtk_signal_connect(GTK_OBJECT(item->entry), "button_press_event",
			GTK_SIGNAL_FUNC(toggle_icon), NULL);
		sprintf(buf, "%i", x);
		gp_progress(100.0*(float)x/(float)count);
		x++;
	}
	gp_progress(0.00);
	gtk_widget_hide(gp_gtk_progress_window);
	gtk_widget_set_sensitive(camera_tree, TRUE);
}

void camera_download_thumbnails() {
	debug_print("camera download thumbnails");

	if (!gp_gtk_camera_init)
		if (camera_set()==GP_ERROR) {return;}

}

void camera_download_photos() {
	debug_print("camera download photos");

	if (!gp_gtk_camera_init)
		if (camera_set()==GP_ERROR) {return;}

}

void camera_download_both() {
	debug_print("camera download both");

	if (!gp_gtk_camera_init)
		if (camera_set()==GP_ERROR) {return;}

}

void camera_delete_common(int all) {

	GtkWidget *icon_list;
	GtkIconListItem *item;
	CameraAbilities a;
	char buf[1024];
	int x, count=0;

	if (!gp_gtk_camera_init)
		if (camera_set()==GP_ERROR) {return;}
	if (all)
		strcpy(buf, _("Are you sure you want to DELETE ALL the photos?"));
	   else
		strcpy(buf, _("Are you sure you want to DELETE the selected photos?"));
		
	if (gp_confirm(buf)==0)
		return;

	if (gp_setting_get("camera", buf)==GP_ERROR) {
		gp_message(_("ERROR: please choose your camera model again"));
		camera_select();
		return;
	}
	if (gp_camera_abilities_by_name(buf, &a)==GP_ERROR) {
		gp_message(_("Could not retrieve the camera's abilities"));
		return;
	}

/* Not working? */
	if (!a.file_delete) {
		gp_message(_("This camera does not support deleting photos."));
		return;
	}

	if ((count = gp_file_count())==GP_ERROR) {
		gp_message(_("Could not retrieve the number of pictures"));
		return;
	}
	
	if (all) {
		for (x=0; x<count; x++) {
			if (gp_file_delete(0)==GP_ERROR) {
				sprintf(buf, _("Could not delete photo #%04i. Stopping."), x);
				gp_message(buf);
				return;
			}
		}
	}
	
	icon_list = (GtkWidget*) lookup_widget(gp_gtk_main_window, "icons");
	for (x=0; x<count; x++) {
		item = gtk_icon_list_get_nth(GTK_ICON_LIST(icon_list), x);
		if (item->state == GTK_STATE_SELECTED) {
			if (gp_file_delete(0)==GP_ERROR) {
				sprintf(buf, _("Could not delete photo #%04i. Stopping."), x);
				gp_message(buf);				
				return;
			}
			gtk_icon_list_remove(GTK_ICON_LIST(icon_list), item);
			x--;
			count--;
		}
	}

	for (x=0; x<count; x++) {
		item = gtk_icon_list_get_nth(GTK_ICON_LIST(icon_list), x);
		sprintf(buf, "Photo #%04i", x);
		gtk_icon_list_set_label (GTK_ICON_LIST(icon_list), item, buf);
		gtk_icon_list_freeze(GTK_ICON_LIST(icon_list));
		gtk_icon_list_thaw(GTK_ICON_LIST(icon_list));
	}
}

void camera_delete_selected() {
	debug_print("camera delete selected");

	camera_delete_common(0);
}

void camera_delete_all() {
	debug_print("camera delete all");

	if (!gp_gtk_camera_init)
		if (camera_set()==GP_ERROR) {return;}

	camera_delete_common(1);
}

void camera_configure() {

	debug_print("camera configure");

	if (!gp_gtk_camera_init)
		if (camera_set()==GP_ERROR) {return;}
}

void camera_show_information() {

	CameraText buf;
	GtkWidget *message, *message_label;

	debug_print("camera information");

	if (!gp_gtk_camera_init)
		if (camera_set()==GP_ERROR) {return;}

	message = create_message_window_transient();
	message_label = (GtkWidget*) lookup_widget(message, "message");
	gtk_label_set_text(GTK_LABEL(message_label), _("Retrieving camera information..."));

	gtk_widget_show(message);
	idle();
	if (gp_summary(&buf)==GP_ERROR) {
		gtk_widget_hide(message);
		gp_message(_("Could not retrieve camera information"));
		return;
	}
	gtk_widget_hide(message);

	gp_message(buf.text);
}

void camera_show_manual() {

	CameraText buf;

	debug_print("camera manual");

	if (!gp_gtk_camera_init)
		if (camera_set()==GP_ERROR) {return;}

	if (gp_manual(&buf)==GP_ERROR) {
		gp_message(_("Could not retrieve the camera manual"));
		return;
	}

	gp_message(buf.text);

}

void camera_show_about() {

	CameraText buf;

	debug_print("camera about");

	if (!gp_gtk_camera_init)
		if (camera_set()==GP_ERROR) {return;}

	if (gp_about(&buf)==GP_ERROR) {
		gp_message(_("Could not retrieve library information"));
		return;
	}

	gp_message(buf.text);
}


/* Help operations */
/* ----------------------------------------------------------- */

void help_about() {

	debug_print("help about");

	gp_message(_("gPhoto2 (v1.90)- Cross-platform digital camera library.\nCopyright (C) 2000 Scott Fritzinger <scottf@unr.edu>\nLicensed under the Library GNU Public License (LGPL)."));
}

void help_authors() {

	char buf[1024];
	CameraFile *f, *g;
	GtkWidget *window, *ok;

	window = create_message_window_notebook();
	ok   = (GtkWidget*) lookup_widget(window, "close");

	debug_print("help authors");

	sprintf(buf, "%s/AUTHORS", DOCDIR);
	f = gp_file_new();
	if (gp_file_open(f, buf)==GP_ERROR) {
		gp_message(_("Can't find gPhoto2 GTK AUTHORS file"));
		return;
	}
	message_window_notebook_append(window, "gPhoto2 GTK Authors", f->data);

	sprintf(buf, "%s/AUTHORS", GPDOCDIR);
	g = gp_file_new();
	if (gp_file_open(g, buf)==GP_ERROR) {
		gp_message(_("Can't find gPhoto2 AUTHORS file"));
		return;
	}
	message_window_notebook_append(window, "gPhoto2 Authors", g->data);

	wait_for_hide(window, ok, NULL);

	gp_file_free(f);
	gp_file_free(g);

	gtk_widget_destroy(window);
}

void help_license() {

	char buf[1024];
	CameraFile *f, *g;
	GtkWidget *window, *ok;

	window = create_message_window_notebook();
	ok   = (GtkWidget*) lookup_widget(window, "close");

	debug_print("help license");

	sprintf(buf, "%s/COPYING", DOCDIR);
	f = gp_file_new();
	if (gp_file_open(f, buf)==GP_ERROR) {
		gp_message(_("Can't find gPhoto2 GTK COPYING file"));
		return;
	}
	message_window_notebook_append(window, "gPhoto2 GTK License", f->data);

	sprintf(buf, "%s/COPYING", GPDOCDIR);
	g = gp_file_new();
	if (gp_file_open(g, buf)==GP_ERROR) {
		gp_message(_("Can't find gPhoto2 COPYING file"));
		return;
	}
	message_window_notebook_append(window, "gPhoto2 License", g->data);

	wait_for_hide(window, ok, NULL);

	gp_file_free(f);
	gp_file_free(g);
	
	gtk_widget_destroy(window);
}

void help_manual() {

	char buf[1024];
	CameraFile *f;
	GtkWidget *window, *ok;

	debug_print("help manual");

	sprintf(buf, "%s/MANUAL", DOCDIR);
	f = gp_file_new();
	if (gp_file_open(f, buf)==GP_ERROR) {
		gp_message(_("Can't find gPhoto2 GTK MANUAL file"));
		return;
	}
	window = create_message_window_notebook();
	ok = (GtkWidget*) lookup_widget(window, "close");
	message_window_notebook_append(window, "gPhoto2 GTK Manual", f->data);

	wait_for_hide(window, ok, NULL);

	gp_file_free(f);
	
	gtk_widget_destroy(window);
}

/* Menu callbacks. just calls above mentioned functions */
void on_open_photo_activate (GtkMenuItem *menuitem, gpointer user_data) {
	open_photo();
}

void on_open_directory_activate (GtkMenuItem *menuitem, gpointer user_data) {
	open_directory();
}

void on_save_photo_activate (GtkMenuItem *menuitem, gpointer user_data) {
	save_selected_photos();
}


void on_html_gallery_activate (GtkMenuItem *menuitem, gpointer user_data) {
	export_gallery();
}

void on_close_activate (GtkMenuItem *menuitem, gpointer user_data) {
	close_photo();
}


void on_exit_activate (GtkMenuItem *menuitem, gpointer user_data) {
	main_quit(NULL, NULL);
}

void on_select_all_activate (GtkMenuItem *menuitem, gpointer user_data) {
	select_all();
}


void on_select_inverse_activate (GtkMenuItem *menuitem, gpointer user_data) {
	select_inverse();
}


void on_select_none_activate (GtkMenuItem *menuitem, gpointer user_data) {
	select_none();
}


void on_select_camera_activate (GtkMenuItem *menuitem, gpointer user_data) {
	camera_select();
}

void on_download_thumbnails_activate (GtkMenuItem *menuitem, gpointer user_data) {
	camera_download_thumbnails();
}


void on_download_photos_activate (GtkMenuItem *menuitem, gpointer user_data) {
	camera_download_photos();
}


void on_download_both_activate (GtkMenuItem *menuitem, gpointer user_data) {
	camera_download_both();
}


void on_delete_photos_activate (GtkMenuItem *menuitem, gpointer user_data) {
	camera_delete_selected();
}


void on_delete_all_activate (GtkMenuItem *menuitem, gpointer user_data) {
	camera_delete_all();
}


void on_configure_activate (GtkMenuItem *menuitem, gpointer user_data) {
	camera_configure();
}


void on_driver_information_activate (GtkMenuItem *menuitem, gpointer user_data) {
	camera_show_information();
}


void on_driver_manual_activate (GtkMenuItem *menuitem, gpointer user_data) {
	camera_show_manual();
}


void on_driver_about_activate (GtkMenuItem *menuitem, gpointer user_data) {
	camera_show_about();
}


void on_manual_activate (GtkMenuItem *menuitem, gpointer user_data) {
	help_manual();
}


void on_authors_activate (GtkMenuItem *menuitem, gpointer user_data) {
	help_authors();
}


void on_license_activate (GtkMenuItem *menuitem, gpointer user_data) {
	help_license();
}


void on_about_activate (GtkMenuItem *menuitem, gpointer user_data) {
	help_about();
}
