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
#include <gal/e-paned/e-hpaned.h>
#include "gphoto-extensions.h"
#include "gnocam.h"
#include "cameras.h"
#include "file-operations.h"
#include "capture.h"

/**********************/
/* External Variables */
/**********************/

extern BonoboUIComponent*	main_component;
extern Bonobo_UIContainer	corba_container;
extern GtkTree*			main_tree;
extern GtkWindow*		main_window;
extern gint			counter;
extern EPaned*			main_paned;

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

void on_tree_item_expand        (GtkTreeItem* tree_item, gpointer user_data);
void on_tree_item_collapse      (GtkTreeItem* tree_item, gpointer user_data);

void on_tree_item_select	(GtkTreeItem* item, gpointer user_data);

void on_drag_data_received                      (GtkWidget* widget, GdkDragContext* context, gint x, gint y, GtkSelectionData* selection_data, guint info, guint time);
void on_camera_tree_file_drag_data_get          (GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data, guint info, guint time, gpointer data);
void on_camera_tree_folder_drag_data_get        (GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data, guint info, guint time, gpointer data);

/*************/
/* Callbacks */
/*************/

void
on_refresh_activate (BonoboUIComponent* component, gpointer folder, const gchar *name)
{
        /* Clean the folder... */
        camera_tree_folder_clean (GTK_TREE_ITEM (folder));
}

void
on_capture_preview_activate (BonoboUIComponent* component, gpointer folder, const gchar* name)
{
	g_return_if_fail (capture_new (gtk_object_get_data (GTK_OBJECT (folder), "camera"), GP_CAPTURE_PREVIEW));
}

void
on_capture_image_activate (BonoboUIComponent* component, gpointer folder, const gchar* name)
{
        g_return_if_fail (capture_new (gtk_object_get_data (GTK_OBJECT (folder), "camera"), GP_CAPTURE_IMAGE));
}

void
on_capture_video_activate (BonoboUIComponent* component, gpointer folder, const gchar* name)
{
	g_return_if_fail (capture_new (gtk_object_get_data (GTK_OBJECT (folder), "camera"), GP_CAPTURE_VIDEO));
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
	Camera*		camera = gtk_object_get_data (GTK_OBJECT (folder), "camera");
	CameraText 	manual;
	gint 		result;

	g_return_if_fail (camera);
	
	if ((result = gp_camera_manual (camera, &manual)) == GP_OK) {
		gnome_ok_dialog_parented (manual.text, main_window);
	} else {
		gchar* tmp = g_strdup_printf (_("Could not get camera manual!\n(%s)"), gp_camera_result_as_string (camera, result));
		gnome_error_dialog_parented (tmp, main_window);
		g_free (tmp);
	}
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
	GtkWidget*		widget;

	g_return_if_fail (item);
	g_return_if_fail (main_paned);

	/* If there is an old viewer, destroy it. */
	if (main_paned->child2) gtk_container_remove (GTK_CONTAINER (main_paned), main_paned->child2);

	/* Display! */
	gtk_widget_show (widget = bonobo_widget_new_control (gtk_object_get_data (GTK_OBJECT (item), "url"), corba_container));
	e_paned_pack2 (main_paned, widget, TRUE, TRUE);
}

void
on_camera_tree_file_drag_data_get (GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data, guint info, guint time, gpointer data)
{
	/* Get the URI of the tree item. */
	gchar* tmp = gnome_vfs_uri_to_string (gtk_object_get_data (GTK_OBJECT (widget), "uri"), GNOME_VFS_URI_HIDE_NONE);
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
        guint                   i;

	/* Upload each file in the URI. */
	GList* filenames = gnome_uri_list_extract_filenames (selection_data->data);
        for (i = 0; i < g_list_length (filenames); i++) upload (GTK_TREE_ITEM (widget), g_list_nth_data (filenames, i));
        gnome_uri_list_free_strings (filenames);
}

/*************/
/* Functions */
/*************/

void
camera_tree_folder_clean (GtkTreeItem* folder)
{
	Bonobo_Storage			storage;
	GtkWidget*			tree;
	gchar*				url;
	Bonobo_Storage_DirectoryList*	list;

	g_return_if_fail (folder);
	g_return_if_fail (url = gtk_object_get_data (GTK_OBJECT (folder), "url"));

	/* Delete all items of subtree. Then, delete the subtree.*/
	if (folder->subtree) {
		gint i;
		for (i = g_list_length (GTK_TREE (folder->subtree)->children) - 1; i >= 0; i--) {
			gtk_container_remove (GTK_CONTAINER (folder->subtree), GTK_WIDGET (g_list_nth_data (GTK_TREE (folder->subtree)->children, i)));
		}
		gtk_tree_item_remove_subtree (folder);
	}

	/* Do we already have the list? */
	if (!(list = gtk_object_get_data (GTK_OBJECT (folder), "list"))) {
		CORBA_Environment ev;

		/* Init exception. */
		CORBA_exception_init (&ev);
	
		/* Do we already have the storage? */
		if (!(storage = gtk_object_get_data (GTK_OBJECT (folder), "storage"))) {
#if 0
			storage = bonobo_get_object (url, "IDL:Bonobo/Storage:1.0", &ev);
			if (BONOBO_EX (&ev)) g_warning (_("Could not get storage for '%s'! (%s)"), url, bonobo_exception_get_text (&ev));
#else
			//FIXME: Above doesn't work. Why???
			storage = BONOBO_OBJREF (bonobo_storage_open_full ("camera", url, 0644, Bonobo_Storage_READ, &ev));
			if (BONOBO_EX (&ev)) g_warning (_("Could not get storage for '%s'! (%s)"), url, bonobo_exception_get_text (&ev));
#endif
		}
		if (storage) {
			gtk_object_set_data_full (GTK_OBJECT (folder), "storage", storage, (GtkDestroyNotify) bonobo_object_unref);
			list = Bonobo_Storage_listContents (storage, "", Bonobo_FIELD_TYPE, &ev);
			if (BONOBO_EX (&ev)) g_warning (_("Could not get list of contents for '%s'!\n(%s)"), url, bonobo_exception_get_text (&ev));
			else gtk_object_set_data (GTK_OBJECT (folder), "list", list);
		}

		/* Free exception. */
		CORBA_exception_free (&ev);
	}

	/* Do we have contents? */
	if (list) {
		if (list->_length > 0) {
			gtk_widget_show (tree = gtk_tree_new ());
			gtk_tree_item_set_subtree (folder, tree);
		}
	}

	gtk_object_set_data (GTK_OBJECT (folder), "populated", NULL);
}

void 
camera_tree_folder_populate (GtkTreeItem* folder)
{
	gchar*				url = gtk_object_get_data (GTK_OBJECT (folder), "url");
	Bonobo_Storage_DirectoryList*	list;
	int 				i;
	gchar*				tmp;

	g_return_if_fail (folder->subtree);
	g_return_if_fail (url);
	g_return_if_fail (list = gtk_object_get_data (GTK_OBJECT (folder), "list"));

	/* Does the folder contain anything? */
	for (i = 0; i < list->_length; i++) {
		switch (list->_buffer [i].type) {
		case Bonobo_STORAGE_TYPE_DIRECTORY:
			if (url[strlen(url) - 1] == '/') tmp = g_strconcat (url, list->_buffer [i].name, NULL);
			else tmp = g_strconcat (url, "/", list->_buffer [i].name, NULL);
			camera_tree_folder_add (GTK_TREE (folder->subtree), NULL, tmp);
			g_free (tmp);
			break;
		case Bonobo_STORAGE_TYPE_REGULAR:
			if (url[strlen(url) - 1] == '/') tmp = g_strconcat (url, list->_buffer [i].name, NULL);
			else tmp = g_strconcat (url, "/", list->_buffer [i].name, NULL);
			camera_tree_file_add (GTK_TREE (folder->subtree), tmp);
			g_free (tmp);
			break;
		default:
			g_assert_not_reached ();
		}
	}

	gtk_object_set_data (GTK_OBJECT (folder), "populated", GINT_TO_POINTER (1));
}

void
camera_tree_folder_add (GtkTree* tree, Camera* camera, gchar* url)
{
	GtkWidget*		item;
	gboolean		root = (camera != NULL);
	GtkTargetEntry 		target_table[] = {{"text/uri-list", 0, 0}};

	g_return_if_fail (tree);
	g_return_if_fail (url);

	if (!camera) g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (tree->tree_owner), "camera"));

	/* Create the item. */
	if (root) gtk_widget_show (item = gtk_tree_item_new_with_label (((frontend_data_t*) camera->frontend_data)->name));
	else gtk_widget_show (item = gtk_tree_item_new_with_label (url));
        gtk_tree_append (tree, item);

	/* The camera is ours. */
	gp_camera_ref (camera);
	gtk_object_set_data_full (GTK_OBJECT (item), "camera", camera, (GtkDestroyNotify) gp_camera_unref);
	
	gtk_object_set_data_full (GTK_OBJECT (item), "url", g_strdup (url), (GtkDestroyNotify) g_free);

        /* For drag and drop. */
        //FIXME: Right now, only drops onto the camera (= root folder) work. Why?!?
        gtk_drag_dest_set (item, GTK_DEST_DEFAULT_ALL, target_table, 1, GDK_ACTION_COPY);
	gtk_drag_source_set (item, GDK_BUTTON1_MASK | GDK_BUTTON3_MASK, target_table, 1, GDK_ACTION_COPY);

        /* Connect the signals. */
        gtk_signal_connect (GTK_OBJECT (item), "drag_data_received", GTK_SIGNAL_FUNC (on_drag_data_received), NULL);
	gtk_signal_connect (GTK_OBJECT (item), "expand", GTK_SIGNAL_FUNC (on_tree_item_expand), NULL);
        gtk_signal_connect (GTK_OBJECT (item), "collapse", GTK_SIGNAL_FUNC (on_tree_item_collapse), NULL);
        gtk_signal_connect (GTK_OBJECT (item), "select", GTK_SIGNAL_FUNC (on_tree_item_select), NULL);
	gtk_signal_connect (GTK_OBJECT (item), "drag_data_get", GTK_SIGNAL_FUNC (on_camera_tree_folder_drag_data_get), NULL);

	/* Clean the item (in order to check if the folder is empty or not). */
	camera_tree_folder_clean (GTK_TREE_ITEM (item));

	/* Prevent this folder from being removed. */
	gtk_object_set_data (GTK_OBJECT (item), "checked", GINT_TO_POINTER (1));
}

void
camera_tree_file_add (GtkTree* tree, gchar* url)
{
	GtkWidget*		item;
	GtkTargetEntry  	target_table[] = {{"text/uri-list", 0, 0}};

	g_return_if_fail (tree);
	g_return_if_fail (url);

	/* Add the file to the tree. */
        gtk_widget_show (item = gtk_tree_item_new_with_label (url));
        gtk_tree_append (tree, item);

	gtk_object_set_data_full (GTK_OBJECT (item), "url", g_strdup (url), (GtkDestroyNotify) g_free);

	/* Drag and Drop */
	gtk_drag_source_set (item, GDK_BUTTON1_MASK | GDK_BUTTON3_MASK, target_table, 1, GDK_ACTION_COPY);

        /* Connect the signals. */
        gtk_signal_connect (GTK_OBJECT (item), "select", GTK_SIGNAL_FUNC (on_tree_item_select), NULL);
	gtk_signal_connect (GTK_OBJECT (item), "drag_data_get", GTK_SIGNAL_FUNC (on_camera_tree_file_drag_data_get), NULL);
}

void
main_tree_update (void)
{
	GConfClient*	client;
        gint            i;

	g_return_if_fail (main_tree);
	g_return_if_fail (client = gconf_client_get_default ());

        /* Mark each camera in the tree as unchecked. */
        for (i = 0; i < g_list_length (main_tree->children); i++) gtk_object_set_data (GTK_OBJECT (g_list_nth_data (main_tree->children, i)), "checked", NULL);

	/* Investigate if all cameras are in the tree. */
	for (i = 0; ; i++) {
		gchar* path;

		/* Check each entry in gconf's database. */
		path = g_strdup_printf ("/apps/" PACKAGE "/camera/%i", i);
		if (gconf_client_dir_exists (client, path, NULL)) {
			Camera*	camera = NULL;
			gint	j;
			gchar* 	model;
			gchar* 	name;
			gchar* 	port;
			gchar* 	tmp;

			tmp = g_strconcat (path, "/model", NULL);
			model = g_strdup (gconf_client_get_string (client, tmp, NULL));
			g_free (tmp);

			tmp = g_strconcat (path, "/port", NULL);
			port = g_strdup (gconf_client_get_string (client, tmp, NULL));
			g_free (tmp);

			tmp = g_strconcat (path, "/name", NULL);
			name = g_strdup (gconf_client_get_string (client, tmp, NULL));
			g_free (tmp);

	                /* Do we have this camera in the tree? */
	                for (j = 0; j < g_list_length (main_tree->children); j++) {
				GtkTreeItem* item = GTK_TREE_ITEM (g_list_nth_data (main_tree->children, j));
	                	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (item), "camera"));
	                        if (i == ((frontend_data_t*) camera->frontend_data)->id) {
					gchar* label;
	
	                                /* We found the camera. Changed? */
					gtk_label_get (GTK_LABEL (GTK_BIN (item)->child), &label);
					if ((strcmp (label, name) != 0) || (strcmp (camera->model, model) != 0) || (strcmp (camera->port->name, port) != 0)) {
	
						/* We simply remove the camera and add a new one to the tree. */
						gtk_container_remove (GTK_CONTAINER (main_tree), GTK_WIDGET (item));
						j = g_list_length (main_tree->children) - 1;
					}
	                        }
	                }
	                if (j == g_list_length (main_tree->children)) {
				gint result;
	
	                        /* We don't have the camera in the tree (yet). */
				camera = NULL;
	                        if ((result = gp_camera_new_by_description (i, name, model, port, &camera)) != GP_OK) {
					gchar* tmp = g_strdup_printf (_("Could not connect to the camera!\n(%s)"), gp_camera_result_as_string (camera, result));
					gnome_error_dialog_parented (tmp, main_window);
					g_free (tmp);
				} else {
					gchar* url = g_strdup_printf ("camera://%s/", name);
					camera_tree_folder_add (main_tree, camera, url);
					g_free (url);
					gp_camera_unref (camera);
				}
	                }

			g_free (model);
			g_free (name);
			g_free (port);
	        } else {
			g_free (path);
			break;
		}
		g_free (path);
	}
	
        /* Delete all unchecked cameras. */
        for (i = g_list_length (main_tree->children) - 1; i >= 0; i--) {
		GtkWidget* item = g_list_nth_data (main_tree->children, i);
        	if (!gtk_object_get_data (GTK_OBJECT (item), "checked")) gtk_container_remove (GTK_CONTAINER (main_tree), item);
        }

	gtk_object_unref (GTK_OBJECT (client));
}

#if 0
void
camera_tree_item_popup_create (GtkTreeItem* item)
{
	Camera*			camera = gtk_object_get_data (GTK_OBJECT (item), "camera");
	CameraWidget*		window_camera = NULL;
	CameraWidget*		window_file = NULL;
	CameraWidget*		window_folder = NULL;
        CORBA_Environment       ev;
        xmlDocPtr               doc;
        xmlNsPtr                ns;
        xmlNodePtr              node, node_child, command;
	gchar*			tmp;
	gint			result, i;
	GnomeVFSURI*		uri = gtk_object_get_data (GTK_OBJECT (item), "uri");
	BonoboUIComponent*	component;
	GtkWidget*		menu = NULL;
	gchar*			file;
	gchar*			folder;

	g_return_if_fail (uri);
	g_return_if_fail (camera);

	folder = gnome_vfs_uri_extract_dirname (uri);
	file = (gchar*) gnome_vfs_uri_get_basename (uri);

        /* Create the component. */
        component = bonobo_ui_component_new_default ();
        bonobo_ui_component_set_container (component, corba_container);
        gtk_object_set_data_full (GTK_OBJECT (item), "component", component, (GtkDestroyNotify) bonobo_object_unref);
	
	/* Ref the camera. */
	gp_camera_ref (camera);
	gtk_object_set_data_full (GTK_OBJECT (component), "camera", camera, (GtkDestroyNotify) gp_camera_unref);

	/* Ref the uri. */
	gnome_vfs_uri_ref (uri);
	gtk_object_set_data_full (GTK_OBJECT (component), "uri", uri, (GtkDestroyNotify) gnome_vfs_uri_unref);

        /* Prepare the popup. */
        doc = xmlNewDoc ("1.0");
        ns = xmlNewGlobalNs (doc, "xxx", "xxx");
        xmlDocSetRootElement (doc, node = xmlNewNode (ns, "Root"));
        xmlAddChild (node, command = xmlNewNode (ns, "commands"));
	xmlAddChild (node, node_child = xmlNewNode (ns, "popups"));
        xmlAddChild (node_child, node = xmlNewNode (ns, "Popup"));
        tmp = g_strdup_printf ("%i", counter);
        xmlSetProp (node, "name", tmp);
        g_free (tmp);

	/* Folder or file? */
	if (gtk_object_get_data (GTK_OBJECT (item), "file")) {

	        /* File configuration? */
	        if ((result = gp_camera_file_config_get (camera, &window_file, folder, file)) == GP_OK) {
	                xmlAddChild (node, node_child = xmlNewNode (ns, "submenu"));
	                xmlSetProp (node_child, "name", "File Configuration");
	                xmlSetProp (node_child, "_label", "File Configuration");
			xmlSetProp (node_child, "_tip", "File configuration");
			popup_prepare (component, window_file, node_child, command, ns);
	                xmlAddChild (node, xmlNewNode (ns, "separator"));
			gtk_object_set_data_full (GTK_OBJECT (item), "window_file", window_file, (GtkDestroyNotify) gp_widget_unref);
	        }
	
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
			if (camera->abilities->config) {
		                if ((result = gp_camera_config_get (camera, &window_camera)) == GP_OK) {
		                        xmlAddChild (node, node_child = xmlNewNode (ns, "submenu"));
		                        xmlSetProp (node_child, "name", "Camera Configuration");
		                        xmlSetProp (node_child, "_label", "Camera Configuration");
					xmlSetProp (node_child, "_tip", "Camera configuration");
		                        popup_prepare (component, window_camera, node_child, command, ns);
					gtk_object_set_data_full (GTK_OBJECT (item), "window_camera", window_camera, (GtkDestroyNotify) gp_widget_unref);
		                } else {
					tmp = g_strdup_printf (_("Could not get camera configuration!\n(%s)"), gp_camera_result_as_string (camera, result));
					bonobo_ui_component_set_status (main_component, tmp, NULL);
					g_free (tmp);
				}
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
	                if (camera->abilities->capture != GP_CAPTURE_NONE) {
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
			xmlSetProp (node_child, "_tip", "Folder configuration");
	                popup_prepare (component, window_folder, node_child, command, ns);
        	        xmlAddChild (node, xmlNewNode (ns, "separator"));
			gtk_object_set_data_full (GTK_OBJECT (item), "window_folder", window_folder, (GtkDestroyNotify) gp_widget_unref);
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
                if (window_camera) {
			tmp = g_strdup_printf ("/popups/%i/Camera Configuration", counter);
			popup_fill (component, tmp, window_camera, window_camera, TRUE);
			g_free (tmp);
		}
                if (window_folder) {
			tmp = g_strdup_printf ("/popups/%i/Folder Configuration", counter);
			popup_fill (component, tmp, window_folder, window_folder, FALSE);
			g_free (tmp);
		}
		if (window_file) {
			tmp = g_strdup_printf ("/popups/%i/File Configuration", counter);
			popup_fill (component, tmp, window_file, window_file, FALSE);
			g_free (tmp);
		}
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

	/* To make sure the _user_ toggled... */
	gtk_object_set_data (GTK_OBJECT (component), "done", GINT_TO_POINTER (1));

	/* Clean up. */
	g_free (folder);
}
#endif

