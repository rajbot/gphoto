/**
 * This file should be called main-tree-utils.c or so.
 * I'll change the name some day.
 * 
 * Should this file be split up? 
 */

#include <config.h>
#include <bonobo.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include <parser.h>
#include <libgnomevfs/gnome-vfs.h>
#include "gphoto-extensions.h"
#include "gnocam.h"
#include "cameras.h"
#include "file-operations.h"
#include "utils.h"
#include "preview.h"

/**********************/
/* External Variables */
/**********************/

extern GConfClient*		gconf_client;
extern Bonobo_UIContainer	corba_container;
extern GtkTree*			main_tree;
extern GnoCamViewMode		view_mode;
extern GList*			preview_list;
extern GtkWindow*		main_window;
extern GladeXML*		xml_main;
extern gint			counter;

/**************/
/* Prototypes */
/**************/

void on_refresh_activate 		(BonoboUIComponent* component, gpointer folder, const gchar* name);
void on_capture_preview_activate 	(BonoboUIComponent* component, gpointer folder, const gchar* name);
void on_capture_image_activate		(BonoboUIComponent* component, gpointer folder, const gchar* name);
void on_capture_video_activate		(BonoboUIComponent* component, gpointer folder, const gchar* name);
void on_upload_activate			(BonoboUIComponent* component, gpointer folder, const gchar* name);
void on_manual_activate 		(BonoboUIComponent* component, gpointer folder, const gchar* name);
void on_save_file_activate		(BonoboUIComponent* component, gpointer folder, const gchar* name);
void on_save_file_as_activate		(BonoboUIComponent* component, gpointer folder, const gchar* name);
void on_save_preview_activate		(BonoboUIComponent* component, gpointer folder, const gchar* name);
void on_save_preview_as_activate	(BonoboUIComponent* component, gpointer folder, const gchar* name);
void on_delete_activate			(BonoboUIComponent* component, gpointer folder, const gchar* name);

gboolean on_tree_item_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data);

void on_tree_item_expand        (GtkTreeItem* tree_item, gpointer user_data);
void on_tree_item_collapse      (GtkTreeItem* tree_item, gpointer user_data);

void on_tree_item_deselect      (GtkTreeItem* item, gpointer user_data);
void on_tree_item_select        (GtkTreeItem* item, gpointer user_data);

void on_drag_data_received                      (GtkWidget* widget, GdkDragContext* context, gint x, gint y, GtkSelectionData* selection_data, guint info, guint time);
void on_camera_tree_file_drag_data_get          (GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data, guint info, guint time, gpointer data);
void on_camera_tree_folder_drag_data_get        (GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data, guint info, guint time, gpointer data);

/*************/
/* Callbacks */
/*************/

void
on_refresh_activate (BonoboUIComponent* component, gpointer folder, const gchar *name)
{
        gboolean populated;

        populated = (gtk_object_get_data (GTK_OBJECT (folder), "populated") != NULL);

        /* Clean the folder... */
        camera_tree_folder_clean (GTK_TREE_ITEM (folder));

        /* ... and fill it. */
        if (populated) camera_tree_folder_populate (GTK_TREE_ITEM (folder));
}

void
on_capture_preview_activate (BonoboUIComponent* component, gpointer folder, const gchar* name)
{
        preview_list = g_list_append (preview_list, preview_new (gtk_object_get_data (GTK_OBJECT (folder), "camera")));
}

void
on_capture_image_activate (BonoboUIComponent* component, gpointer folder, const gchar* name)
{
        capture_image (gtk_object_get_data (GTK_OBJECT (folder), "camera"));
}

void
on_capture_video_activate (BonoboUIComponent* component, gpointer folder, const gchar* name)
{
        capture_video (gtk_object_get_data (GTK_OBJECT (folder), "camera"));
}

void
on_upload_activate (BonoboUIComponent* component, gpointer folder, const gchar* name)
{
	upload (GTK_TREE_ITEM (folder), NULL);
}

void
on_save_preview_activate (BonoboUIComponent* component, gpointer file, const gchar* name)
{
	save (GTK_TREE_ITEM (file), TRUE);
}

void
on_save_preview_as_activate (BonoboUIComponent* component, gpointer file, const gchar* name)
{
	download (GTK_TREE_ITEM (file), NULL, TRUE);
}

void
on_delete_activate (BonoboUIComponent* component, gpointer file, const gchar* name)
{
	delete (GTK_TREE_ITEM (file));
}

void
on_save_file_activate (BonoboUIComponent* component, gpointer file, const gchar* name)
{
	save (GTK_TREE_ITEM (file), FALSE);
}

void
on_save_file_as_activate (BonoboUIComponent* component, gpointer file, const gchar* name)
{
	download (GTK_TREE_ITEM (file), NULL, FALSE);
}

void
on_manual_activate (BonoboUIComponent* component, gpointer folder, const gchar* name)
{
	Camera*		camera;
	CameraText 	manual;
	gint 		result;
	gchar*		tmp;

	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (folder), "camera"));
	
	if ((result = gp_camera_manual (camera, &manual)) == GP_OK) {
		gnome_ok_dialog_parented (manual.text, main_window);
	} else {
		tmp = g_strdup_printf (_("Could not get camera manual!\n(%s)"), gp_camera_result_as_string (camera, result));
		gnome_error_dialog_parented (tmp, main_window);
		g_free (tmp);
	}
}

gboolean
on_tree_item_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	GtkMenu*	menu;

        /* Did the user right-click? */
        if (event->button != 3) return (FALSE);

	/* If needed, create the popup. */
	if (!(menu = gtk_object_get_data (GTK_OBJECT (widget), "menu"))) camera_tree_item_popup_create (GTK_TREE_ITEM (widget));
        gtk_menu_popup (gtk_object_get_data (GTK_OBJECT (widget), "menu"), NULL, NULL, NULL, NULL, 3, 0);
        return (TRUE);
}

void
on_tree_item_expand (GtkTreeItem* tree_item, gpointer user_data)
{
        if (!gtk_object_get_data (GTK_OBJECT (tree_item), "populated")) camera_tree_folder_populate (tree_item);
}

void
on_tree_item_collapse (GtkTreeItem* tree_item, gpointer user_data)
{
        /* We currently don't do anything here. */
}

void
on_tree_item_select (GtkTreeItem* item, gpointer user_data)
{
	gchar*			tmp = NULL;
	CORBA_Environment	ev;
	Bonobo_Control		control;
	GtkWidget*		widget;
	GtkPaned*		paned;
	GnomeVFSURI*		uri;

	g_return_if_fail (item);
	g_return_if_fail (paned = GTK_PANED (glade_xml_get_widget (xml_main, "main_hpaned")));

	/* We don't display anything if a folder gets selected or if the user selected GNOCAM_VIEW_MODE_NONE. */
	if (item->subtree || view_mode == GNOCAM_VIEW_MODE_NONE) return;

	/* Init exceptions. */
	CORBA_exception_init (&ev);

	/* If there is an old viewer, destroy it. */
	if (paned->child2) gtk_container_remove (GTK_CONTAINER (paned), paned->child2);

//FIXME: Until monikers can resolve "camera:", we first have to save the file.

	/* Save the file temporarily. */
	tmp = g_strdup_printf ("file:%s/%s", g_get_tmp_dir (), gnome_vfs_uri_get_basename (gtk_object_get_data (GTK_OBJECT (item), "uri")));
	uri = gnome_vfs_uri_new (tmp);
	download (item, uri, (view_mode == GNOCAM_VIEW_MODE_PREVIEW));
	gnome_vfs_uri_unref (uri);

	/* Get the control. */
	control = bonobo_get_object (tmp, "IDL:Bonobo/Control:1.0", &ev);
	g_free (tmp);
	if (BONOBO_EX (&ev)) {
		tmp = g_strdup_printf (_("Could not get any widget for\ndisplaying the contents of the file!\n(%s)"), bonobo_exception_get_text (&ev));
		gnome_error_dialog_parented (tmp, main_window);
		g_free (tmp);
		CORBA_exception_free (&ev);
		return;
	}
	widget = bonobo_widget_new_control_from_objref (control, corba_container);
	gtk_widget_show (widget);
	gtk_paned_pack2 (paned, widget, TRUE, TRUE);

	/* Clean up. */
	CORBA_exception_free (&ev);
}

void
on_tree_item_deselect (GtkTreeItem* item, gpointer user_data)
{
	/* Nothing to do here. */
}

void
on_camera_tree_file_drag_data_get (GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data, guint info, guint time, gpointer data)
{
	gchar*		tmp;

	/* Get the URI of the tree item. */
	tmp = gnome_vfs_uri_to_string (gtk_object_get_data (GTK_OBJECT (widget), "uri"), GNOME_VFS_URI_HIDE_NONE);
	gtk_selection_data_set (selection_data, selection_data->target, 8, tmp, strlen (tmp));
}

void
on_camera_tree_folder_drag_data_get (GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data, guint info, guint time, gpointer data)
{
	gnome_ok_dialog (_("Not yet implemented."));
}

void
on_drag_data_received (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint time)
{
        GList*                  filenames = NULL;
        guint                   i;
	Camera*			camera;

	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (widget), "camera"));

	/* Upload each file in the URI. */
        filenames = gnome_uri_list_extract_filenames (selection_data->data);
        for (i = 0; i < g_list_length (filenames); i++) upload (GTK_TREE_ITEM (widget), g_list_nth_data (filenames, i));
        gnome_uri_list_free_strings (filenames);
}

/*************/
/* Functions */
/*************/

void
camera_tree_folder_clean (GtkTreeItem* folder)
{
	gint		i;

	g_return_if_fail (folder);
	g_return_if_fail (folder->subtree);

	/* Delete all items of tree. */
	for (i = g_list_length (GTK_TREE (folder->subtree)->children) - 1; i >= 0; i--) 
		camera_tree_item_remove (GTK_TREE_ITEM (g_list_nth_data (GTK_TREE (folder->subtree)->children, i)));
	gtk_object_set_data (GTK_OBJECT (folder), "populated", NULL);
}

void 
camera_tree_folder_populate (GtkTreeItem* folder)
{
	GnomeVFSURI*		uri;
	gchar*			path;
	gchar*			tmp;
        gint                    i;
	Bonobo_Storage_DirectoryList* 	list;
	CORBA_Environment		ev;

	g_return_if_fail (folder->subtree);
	g_return_if_fail (uri = gtk_object_get_data (GTK_OBJECT (folder), "uri"));

	CORBA_exception_init (&ev);
	list = Bonobo_Storage_listContents (gtk_object_get_data (GTK_OBJECT (folder), "corba_storage"), "", Bonobo_FIELD_TYPE, &ev);
	if (!BONOBO_EX (&ev)) {
		for (i = 0; i < list->_length; i++) {
			switch (list->_buffer [i].type) {
			case Bonobo_STORAGE_TYPE_DIRECTORY:
				camera_tree_folder_add (GTK_TREE (folder->subtree), NULL, gnome_vfs_uri_append_path (uri, list->_buffer [i].name));
				break;
			case Bonobo_STORAGE_TYPE_REGULAR:
				camera_tree_file_add (GTK_TREE (folder->subtree), gnome_vfs_uri_append_file_name (uri, list->_buffer [i].name));
				break;
			default:
				g_assert_not_reached ();
			}
		}
		//FIXME: Do I have to free 'list'?
	} else {
		path = gnome_vfs_unescape_string_for_display (gnome_vfs_uri_get_path (uri));
		tmp = g_strdup_printf ("Could not get contents of folder '%s'!\n(%s)", path, bonobo_exception_get_text (&ev));
		g_free (path);
		gnome_error_dialog_parented (tmp, main_window);
		g_free (tmp);
	}
	CORBA_exception_free (&ev);

	gtk_object_set_data (GTK_OBJECT (folder), "populated", GINT_TO_POINTER (1));
}

void
camera_tree_item_remove (GtkTreeItem* item)
{
	BonoboUIComponent*	component;
	CameraWidget*		window;
	GtkWidget*		owner;
	GtkTree*		tree;
	CORBA_Environment	ev;

	g_return_if_fail (item);
	g_return_if_fail (tree = GTK_TREE (GTK_WIDGET (item)->parent));

	/* Unref the camera and the URI. */
	gp_camera_unref (gtk_object_get_data (GTK_OBJECT (item), "camera"));
	gnome_vfs_uri_unref (gtk_object_get_data (GTK_OBJECT (item), "uri"));

	/* Unref the component. */
	if ((component = gtk_object_get_data (GTK_OBJECT (item), "component"))) {
		gp_camera_unref (gtk_object_get_data (GTK_OBJECT (component), "camera"));
	}

	if ((window = gtk_object_get_data (GTK_OBJECT (item), "window_camera"))) gp_widget_unref (window);
	if ((window = gtk_object_get_data (GTK_OBJECT (item), "window_folder"))) gp_widget_unref (window);
	if ((window = gtk_object_get_data (GTK_OBJECT (item), "window_file"))) gp_widget_unref (window);
	
	/* Unref the storage. */
	CORBA_exception_init (&ev);
	Bonobo_Storage_unref (gtk_object_get_data (GTK_OBJECT (item), "corba_storage"), &ev);
	CORBA_exception_free (&ev);

	/* If item is a folder, clean it. */
	if (item->subtree) {
		camera_tree_folder_clean (item);
		gtk_widget_unref (item->subtree);
	}

	/* If this is the last item, we have to make sure we don't loose the 	*/
	/* tree. Therefore, keep a reference to the tree owner. 		*/
	owner = tree->tree_owner;

	/* Get rid of the tree item. */
        gtk_container_remove (GTK_CONTAINER (tree), GTK_WIDGET (item));

	/* Make sure the tree does not get lost. */
	if (GTK_WIDGET (tree)->parent == NULL) {

		/* This does only happen with tree items. I hope so. */
		g_assert (GTK_IS_TREE_ITEM (owner));
		gtk_tree_item_set_subtree (GTK_TREE_ITEM (owner), GTK_WIDGET (tree));
	}
}

void
camera_tree_folder_add (GtkTree* tree, Camera* camera, GnomeVFSURI* uri)
{
	GtkWidget*		item;
	GtkWidget*		widget;
	gboolean		root;
	gchar*			path;
	gchar*			tmp;
	GtkTargetEntry 		target_table[] = {{"text/uri-list", 0, 0}};
	CORBA_Environment	ev;
	BonoboStorage*		storage = NULL;
	Bonobo_Storage		corba_storage = CORBA_OBJECT_NIL;

	g_return_if_fail (tree);
	g_return_if_fail (uri);

	/* Root items (camera != NULL) differ from non-root items. */
	root = (camera != NULL);
	if (!camera) g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (tree->tree_owner), "camera"));

	/* Create the storage. First try "rw", then "r". */
	if (root) {
		tmp = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
		CORBA_exception_init (&ev);
		storage = bonobo_storage_open_full ("camera", tmp, 0664, Bonobo_Storage_READ | Bonobo_Storage_WRITE, &ev);
		if (BONOBO_EX (&ev)) {
			CORBA_exception_free (&ev);
			CORBA_exception_init (&ev);
			storage = bonobo_storage_open_full ("camera", tmp, 0664, Bonobo_Storage_READ, &ev);
		}
		g_free (tmp);
	} else {
		tmp = gnome_vfs_unescape_string_for_display (gnome_vfs_uri_get_basename (uri));
		CORBA_exception_init (&ev);
		corba_storage = Bonobo_Storage_openStorage (
			gtk_object_get_data (GTK_OBJECT (tree->tree_owner), "corba_storage"), tmp, Bonobo_Storage_READ | Bonobo_Storage_WRITE, &ev);
		if (BONOBO_EX (&ev)) {
			CORBA_exception_free (&ev);
			CORBA_exception_init (&ev);
			corba_storage = Bonobo_Storage_openStorage (
				gtk_object_get_data (GTK_OBJECT (tree->tree_owner), "corba_storage"), tmp, Bonobo_Storage_READ, &ev);
		}
		g_free (tmp);
	}
	if (BONOBO_EX (&ev)) {
		path = gnome_vfs_unescape_string_for_display (gnome_vfs_uri_get_path (uri));
		tmp = g_strdup_printf (_("Could not get storage for folder '%s'!\n(%s)"), path, bonobo_exception_get_text (&ev));
		g_free (path);
		gnome_error_dialog_parented (tmp, main_window);
		g_free (tmp);
		CORBA_exception_free (&ev);
		return;
	}
	if (root) corba_storage = bonobo_storage_corba_object_create (BONOBO_OBJECT (storage));
	CORBA_exception_free (&ev);

	/* The camera is ours. */
	gp_camera_ref (camera);
	
	/* Create the item. */
	if (root) {
		item = gtk_tree_item_new_with_label (((frontend_data_t*) camera->frontend_data)->name);
		((frontend_data_t*) camera->frontend_data)->item = GTK_TREE_ITEM (item);
	} else {
		tmp = gnome_vfs_unescape_string_for_display (gnome_vfs_uri_get_basename (uri));
		item = gtk_tree_item_new_with_label (tmp);
		g_free (tmp);
	}
        gtk_widget_show (item);
        gtk_tree_append (tree, item);

        /* For drag and drop. */
        //FIXME: Right now, only drops onto the camera (= root folder) work. Why?!?
        gtk_drag_dest_set (item, GTK_DEST_DEFAULT_ALL, target_table, 1, GDK_ACTION_COPY);
	gtk_drag_source_set (item, GDK_BUTTON1_MASK | GDK_BUTTON3_MASK, target_table, 1, GDK_ACTION_COPY);

        /* Connect the signals. */
        gtk_signal_connect (GTK_OBJECT (item), "drag_data_received", GTK_SIGNAL_FUNC (on_drag_data_received), NULL);
	gtk_signal_connect (GTK_OBJECT (item), "expand", GTK_SIGNAL_FUNC (on_tree_item_expand), NULL);
        gtk_signal_connect (GTK_OBJECT (item), "collapse", GTK_SIGNAL_FUNC (on_tree_item_collapse), NULL);
        gtk_signal_connect (GTK_OBJECT (item), "select", GTK_SIGNAL_FUNC (on_tree_item_select), NULL);
        gtk_signal_connect (GTK_OBJECT (item), "deselect", GTK_SIGNAL_FUNC (on_tree_item_deselect), NULL);
	gtk_signal_connect (GTK_OBJECT (item), "drag_data_get", GTK_SIGNAL_FUNC (on_camera_tree_folder_drag_data_get), NULL);
	gtk_signal_connect (GTK_OBJECT (item), "button_press_event", GTK_SIGNAL_FUNC (on_tree_item_button_press_event), NULL);

        /* Create the subtree. Don't populate it yet. */
        gtk_widget_show (widget = gtk_tree_new ());
        gtk_widget_ref (widget);
        gtk_tree_item_set_subtree (GTK_TREE_ITEM (item), widget);

        /* Store some data. */
        gtk_object_set_data (GTK_OBJECT (item), "camera", camera);
        gtk_object_set_data (GTK_OBJECT (item), "uri", uri);
        gtk_object_set_data (GTK_OBJECT (item), "corba_storage", corba_storage);
        gtk_object_set_data (GTK_OBJECT (item), "checked", GINT_TO_POINTER (1));
}

void
camera_tree_file_add (GtkTree* tree, GnomeVFSURI* uri)
{
	Camera*			camera;
	GtkWidget*		item;
	GtkTargetEntry  	target_table[] = {{"text/uri-list", 0, 0}};
	Bonobo_Storage		corba_storage;
	CORBA_Environment	ev;

	g_return_if_fail (tree);
	g_return_if_fail (uri);
	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (tree->tree_owner), "camera"));
	g_return_if_fail (corba_storage = gtk_object_get_data (GTK_OBJECT (tree->tree_owner), "corba_storage"));

	/* Ref the camera. */
	gp_camera_ref (camera);

	/* Ref the storage. */
	CORBA_exception_init (&ev);
	Bonobo_Storage_ref (corba_storage, &ev);
	CORBA_exception_free (&ev);
	
	/* Add the file to the tree. */
        item = gtk_tree_item_new_with_label (gnome_vfs_uri_get_basename (uri));
        gtk_widget_show (item);
        gtk_tree_append (tree, item);

	/* Drag and Drop */
	gtk_drag_source_set (item, GDK_BUTTON1_MASK | GDK_BUTTON3_MASK, target_table, 1, GDK_ACTION_COPY);

        /* Connect the signals. */
        gtk_signal_connect (GTK_OBJECT (item), "select", GTK_SIGNAL_FUNC (on_tree_item_select), NULL);
        gtk_signal_connect (GTK_OBJECT (item), "deselect", GTK_SIGNAL_FUNC (on_tree_item_deselect), NULL);
        gtk_signal_connect (GTK_OBJECT (item), "button_press_event", GTK_SIGNAL_FUNC (on_tree_item_button_press_event), NULL);
	gtk_signal_connect (GTK_OBJECT (item), "drag_data_get", GTK_SIGNAL_FUNC (on_camera_tree_file_drag_data_get), NULL);

        /* Store some data. */
        gtk_object_set_data (GTK_OBJECT (item), "camera", camera);
        gtk_object_set_data (GTK_OBJECT (item), "uri", uri);
        gtk_object_set_data (GTK_OBJECT (item), "corba_storage", corba_storage);
	gtk_object_set_data (GTK_OBJECT (item), "file", GINT_TO_POINTER (1));
}

void
main_tree_update (GConfValue* value)
{
	xmlDocPtr	doc;
	xmlNodePtr	node;
        GSList*         list_cameras = NULL;
	GtkTreeItem*	item;
	gchar*		xml;
        gchar*          id;
	gchar*		label;
	gchar*		name;
	gchar*		model;
	gchar*		port;
	gchar*		speed;
	gchar*		tmp;
        gint            i;
        gint            j;
	gint		result;
        Camera*         camera;
	gboolean	changed;

	g_return_if_fail (main_tree);

        if (value) {
                g_assert (value->type == GCONF_VALUE_LIST);
                g_assert (gconf_value_get_list_type (value) == GCONF_VALUE_STRING);
                list_cameras = gconf_value_get_list (value);
        }

        /* Mark each camera in the tree as unchecked. */
        for (i = 0; i < g_list_length (main_tree->children); i++) 
		gtk_object_set_data (GTK_OBJECT (g_list_nth_data (main_tree->children, i)), "checked", GINT_TO_POINTER (0));

	/* Investigate if the new cameras are in the tree. */
        for (i = 0; i < g_slist_length (list_cameras); i++) {
                value = g_slist_nth_data (list_cameras, i);
                g_assert (value->type == GCONF_VALUE_STRING);
		g_assert ((xml = g_strdup (gconf_value_get_string (value))) != NULL);
		if (!(doc = xmlParseMemory (g_strdup (xml), strlen (xml)))) continue;
		g_assert ((node = xmlDocGetRootElement (doc)) != NULL);

		/* This sanity check only seems to work with libxml2. */
#ifdef GNOCAM_USES_LIBXML2
		if (strcmp (node->name, "Camera") != 0) continue;
#endif
		g_assert ((id = xmlGetProp (node, "ID")) != NULL);
		g_assert ((name = xmlGetProp (node, "Name")) != NULL);
		g_assert ((model = xmlGetProp (node, "Model")) != NULL);
		g_assert ((port = xmlGetProp (node, "Port")) != NULL);
		g_assert ((speed = xmlGetProp (node, "Speed")) != NULL);

                /* Do we have this camera in the tree? */
                for (j = 0; j < g_list_length (main_tree->children); j++) {
			item = GTK_TREE_ITEM (g_list_nth_data (main_tree->children, j));
                        g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);
                        if (atoi (id) == ((frontend_data_t*) camera->frontend_data)->id) {

                                /* We found the camera. Changed? */
				gtk_label_get (GTK_LABEL (GTK_BIN (item)->child), &label);
				changed = (strcmp (label, name) != 0);
				changed = changed || (strcmp (camera->model, model) != 0);
				changed = changed || (strcmp (camera->port->name, port) != 0);
				changed = changed || (atoi (speed) != camera->port->speed);
				if (changed) {

					/* We simply remove the camera and add a new one to the tree. */
					camera_tree_item_remove (GTK_TREE_ITEM (item));
					j = g_list_length (main_tree->children) - 1;
				}
                        }
                }
                if (j == g_list_length (main_tree->children)) {

                        /* We don't have the camera in the tree (yet). */
			camera = NULL;
                        if ((result = gp_camera_new_by_description (atoi (id), name, model, port, atoi (speed), &camera)) != GP_OK) {
				tmp = g_strdup_printf (_("Could not connect to the camera!\n(%s)"), gp_camera_result_as_string (camera, result));
				gnome_error_dialog_parented (tmp, main_window);
				g_free (tmp);
			} else {
				tmp = g_strdup_printf ("camera://%s/", name);
				camera_tree_folder_add (main_tree, camera, gnome_vfs_uri_new (tmp));
				g_free (tmp);
				gp_camera_unref (camera);
			}
                }
        }

        /* Delete all unchecked cameras. */
        for (j = g_list_length (main_tree->children) - 1; j >= 0; j--) {
                item = GTK_TREE_ITEM (g_list_nth_data (main_tree->children, j));
                if (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item), "checked")) == 0) camera_tree_item_remove (item);
        }
}

void
camera_tree_item_popup_create (GtkTreeItem* item)
{
	Camera*			camera;
	CameraWidget*		window = NULL;
	CameraWidget*		window_folder = NULL;
        CORBA_Environment       ev;
        xmlDocPtr               doc;
        xmlNsPtr                ns;
        xmlNodePtr              node, node_child, command;
	gchar*			tmp;
	gint			result, i;
	GnomeVFSURI*		uri;
	BonoboUIComponent*	component;
	GtkWidget*		menu = NULL;

	g_return_if_fail (uri = gtk_object_get_data (GTK_OBJECT (item), "uri"));
	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (item), "camera"));

        /* Create the component. */
        component = bonobo_ui_component_new_default ();
        bonobo_ui_component_set_container (component, corba_container);
        gtk_object_set_data (GTK_OBJECT (item), "component", component);
        gtk_object_set_data (GTK_OBJECT (component), "camera", camera);
        gp_camera_ref (camera);

        /* Prepare the popup. */
        doc = xmlNewDoc ("1.0");
        ns = xmlNewGlobalNs (doc, "xxx", "xxx");
        xmlDocSetRootElement (doc, node = xmlNewNode (ns, "Root"));
        xmlAddChild (node, command = xmlNewNode (ns, "commands"));
        xmlAddChild (node, node_child = xmlNewNode (ns, "popups"));
        xmlAddChild (node_child, node = xmlNewNode (ns, "popup"));
        tmp = g_strdup_printf ("%i", counter);
        xmlSetProp (node, "name", tmp);
        g_free (tmp);

	/* Folder or file? */
	if (gtk_object_get_data (GTK_OBJECT (item), "file")) {

	        /* File configuration? */
	        tmp = gnome_vfs_uri_extract_dirname (uri);
	        if ((result = gp_camera_file_config_get (camera, &window, tmp, (gchar*) gnome_vfs_uri_get_basename (uri))) == GP_OK) {
	                xmlAddChild (node, node_child = xmlNewNode (ns, "submenu"));
	                xmlSetProp (node_child, "name", "File Configuration");
	                xmlSetProp (node_child, "_label", "File Configuration");
	                gp_widget_to_xml (component, window, window, node_child, command, ns);
	                xmlAddChild (node, xmlNewNode (ns, "separator"));
	        }
	        g_free (tmp);
	
	        /* Save preview? */
	        if (camera->abilities->file_preview) {
	                xmlAddChild (node, node_child = xmlNewNode (ns, "menuitem"));
	                xmlSetProp (node_child, "name", "Save Preview");
	                xmlSetProp (node_child, "_label", "Save Preview");
	                xmlSetProp (node_child, "_tip", "Save preview");
	                xmlSetProp (node_child, "verb", "on_save_preview_activate");
	                bonobo_ui_component_add_verb (component, "on_save_preview_activate", on_save_preview_activate, item);
	                xmlAddChild (node, node_child = xmlNewNode (ns, "menuitem"));
	                xmlSetProp (node_child, "name", "Save Preview As");
	                xmlSetProp (node_child, "_label", "Save Preview As");
	                xmlSetProp (node_child, "_tip", "Save preview as");
	                xmlSetProp (node_child, "verb", "on_save_preview_as_activate");
	                bonobo_ui_component_add_verb (component, "on_save_preview_as_activate", on_save_preview_as_activate, item);
	                xmlAddChild (node, xmlNewNode (ns, "separator"));
	        }
	
	        /* Save file. */
	        xmlAddChild (node, node_child = xmlNewNode (ns, "menuitem"));
	        xmlSetProp (node_child, "name", "Save File");
	        xmlSetProp (node_child, "verb", "on_save_file_activate");
	        xmlSetProp (node_child, "_label", "Save File");
	        xmlSetProp (node_child, "_tip", "Save file");
	        bonobo_ui_component_add_verb (component, "on_save_file_activate", on_save_file_activate, item);
	        xmlAddChild (node, node_child = xmlNewNode (ns, "menuitem"));
	        xmlSetProp (node_child, "name", "Save File As");
	        xmlSetProp (node_child, "verb", "on_save_file_as_activate");
	        xmlSetProp (node_child, "_label", "Save File As");
	        xmlSetProp (node_child, "_tip", "Save file as");
	        bonobo_ui_component_add_verb (component, "on_save_file_as_activate", on_save_file_as_activate, item);
	
	        /* Delete? */
	        if (camera->abilities->file_delete) {
			xmlAddChild (node, xmlNewNode (ns, "separator"));
	                xmlAddChild (node, node_child = xmlNewNode (ns, "menuitem"));
	                xmlSetProp (node_child, "name", "Delete");
	                xmlSetProp (node_child, "_label", "Delete");
	                xmlSetProp (node_child, "_tip", "Delete a file");
	                xmlSetProp (node_child, "verb", "on_delete_activate");
	                bonobo_ui_component_add_verb (component, "on_delete_activate", on_delete_activate, item);
	        }
	} else {

		/* Root? */
		if (strcmp (gnome_vfs_uri_get_path (uri), "/") == 0) {

	                /* Camera configuration? */
	                if ((result = gp_camera_config_get (camera, &window)) == GP_OK) {
	                        xmlAddChild (node, node_child = xmlNewNode (ns, "submenu"));
	                        xmlSetProp (node_child, "name", "Camera Configuration");
	                        xmlSetProp (node_child, "_label", "Camera Configuration");
	                        gp_widget_to_xml (component, window, window, node_child, command, ns);
	                }
	
	                /* Manual. */
	                xmlAddChild (node, node_child = xmlNewNode (ns, "menuitem"));
	                xmlSetProp (node_child, "name", "Manual");
	                xmlSetProp (node_child, "_label", "Manual");
	                xmlSetProp (node_child, "_tip", "Manual");
	                xmlSetProp (node_child, "verb", "on_manual_activate");
	                bonobo_ui_component_add_verb (component, "on_manual_activate", on_manual_activate, item);
	                xmlAddChild (node, xmlNewNode (ns, "separator"));

        	        /* Capturing? */
	                if (camera->abilities->capture) {
	                        if (camera->abilities->capture & GP_CAPTURE_PREVIEW) {
	                                xmlAddChild (node, node_child = xmlNewNode (ns, "menuitem"));
	                                xmlSetProp (node_child, "name", "Capture Preview");
	                                xmlSetProp (node_child, "_label", "Capture Preview");
	                                xmlSetProp (node_child, "_tip", "Capture a preview");
	                                xmlSetProp (node_child, "verb", "on_capture_preview_activate");
	                                bonobo_ui_component_add_verb (component, "on_capture_preview_activate", on_capture_preview_activate, item);
	                        }
	                        if (camera->abilities->capture & GP_CAPTURE_IMAGE) {
	                                xmlAddChild (node, node_child = xmlNewNode (ns, "menuitem"));
	                                xmlSetProp (node_child, "name", "Capture Image");
	                                xmlSetProp (node_child, "_label", "Capture Image");
	                                xmlSetProp (node_child, "_tip", "Capture an image");
	                                xmlSetProp (node_child, "verb", "on_capture_image_activate");
	                                bonobo_ui_component_add_verb (component, "on_capture_image_activate", on_capture_image_activate, item);
	                        }
	                        if (camera->abilities->capture & GP_CAPTURE_VIDEO) {
	                                xmlAddChild (node, node_child = xmlNewNode (ns, "menuitem"));
	                                xmlSetProp (node_child, "name", "Capture Video");
	                                xmlSetProp (node_child, "_label", "Capture Image");
	                                xmlSetProp (node_child, "_tip", "Capture a video");
	                                xmlSetProp (node_child, "verb", "on_capture_video_activate");
	                                bonobo_ui_component_add_verb (component, "on_capture_video_activate", on_capture_video_activate, item);
	                        }
	                        xmlAddChild (node, xmlNewNode (ns, "separator"));
	                }
		}

	        /* Folder configuration? */
	        if ((result = gp_camera_folder_config_get (camera, &window_folder, "/")) == GP_OK) {
	                xmlAddChild (node, node_child = xmlNewNode (ns, "submenu"));
	                xmlSetProp (node_child, "name", "Folder Configuration");
	                xmlSetProp (node_child, "_label", "Folder Configuration");
	                gp_widget_to_xml (component, window_folder, window_folder, node_child, command, ns);
        	        xmlAddChild (node, xmlNewNode (ns, "separator"));
	        }
	
	        /* Upload? */
	        if (camera->abilities->file_put) {
	                xmlAddChild (node, node_child = xmlNewNode (ns, "menuitem"));
	                xmlSetProp (node_child, "name", "Upload");
	                xmlSetProp (node_child, "_label", "Upload");
	                xmlSetProp (node_child, "_tip", "Upload a file");
	                xmlSetProp (node_child, "verb", "on_upload_activate");
	                bonobo_ui_component_add_verb (component, "on_upload_activate", on_upload_activate, item);
	                xmlAddChild (node, xmlNewNode (ns, "separator"));
	        }
	
	        /* Refresh. */
	        xmlAddChild (node, node_child = xmlNewNode (ns, "menuitem"));
	        xmlSetProp (node_child, "name", "Refresh");
	        xmlSetProp (node_child, "verb", "on_refresh_activate");
	        xmlSetProp (node_child, "_label", "Refresh");
	        xmlSetProp (node_child, "_tip", "Refresh folder");
	        bonobo_ui_component_add_verb (component, "on_refresh_activate", on_refresh_activate, item);
	}
	
        /* Finish the popup. */
	CORBA_exception_init (&ev);
        tmp = NULL;
        xmlDocDumpMemory (doc, (xmlChar**) &tmp, &i);
        xmlFreeDoc (doc);
        bonobo_ui_component_set_translate (component, "/", tmp, &ev);
        g_free (tmp);
        if (!BONOBO_EX (&ev)) {
                gtk_widget_show (menu = gtk_menu_new ());
                tmp = g_strdup_printf ("/popups/%i", counter);
                bonobo_window_add_popup (BONOBO_WINDOW (main_window), GTK_MENU (menu), tmp);
                g_free (tmp);
                if (window) ui_set_values_from_widget (component, window, window);
                if (window_folder) ui_set_values_from_widget (component, window_folder, window_folder);
                counter++;
        }

        /* Report errors (if any). */
        if (BONOBO_EX (&ev)) {
                tmp = g_strdup_printf (_("Could not create popup for '%s'!\n(%s)"), gnome_vfs_uri_get_path (uri), bonobo_exception_get_text (&ev));
                gnome_error_dialog_parented (tmp, main_window);
                g_free (tmp);
        }

        /* Free exception. */
        CORBA_exception_free (&ev);

        /* Store some data. */
        gtk_object_set_data (GTK_OBJECT (item), "menu", menu);
	gtk_object_set_data (GTK_OBJECT (item), "window_folder", window_folder);
	gtk_object_set_data (GTK_OBJECT (item), "window", window);

	/* To make sure the _user_ toggled... */
	gtk_object_set_data (GTK_OBJECT (component), "done", GINT_TO_POINTER (1));
}



