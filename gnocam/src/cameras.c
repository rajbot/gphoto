/**
 * This file should be called main-tree-utils.c or so.
 * I'll change the name some day.
 * 
 * Should this file be split up? 
 */

#include <config.h>
#include <gphoto2.h>
#include <bonobo.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <parser.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gal/e-paned/e-hpaned.h>

#include <gphoto-extensions.h>

#include "cameras.h"
#include "file-operations.h"

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

void on_upload_activate			(BonoboUIComponent* component, gpointer folder, const gchar* name);

void on_drag_data_received                      (GtkWidget* widget, GdkDragContext* context, gint x, gint y, GtkSelectionData* selection_data, guint info, guint time);
void on_camera_tree_file_drag_data_get          (GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data, guint info, guint time, gpointer data);
void on_camera_tree_folder_drag_data_get        (GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data, guint info, guint time, gpointer data);

/*************/
/* Callbacks */
/*************/

void
on_upload_activate (BonoboUIComponent* component, gpointer folder, const gchar* name)
{
	upload (GTK_TREE_ITEM (folder), NULL);
}

void
on_camera_tree_file_drag_data_get (GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data, guint info, guint time, gpointer data)
{
	/* Get the URI of the tree item. */
	gchar* tmp = gnome_vfs_uri_to_string (gtk_object_get_data (GTK_OBJECT (widget), "uri"), GNOME_VFS_URI_HIDE_NONE);
	gtk_selection_data_set (selection_data, selection_data->target, 8, tmp, strlen (tmp));
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

#if 0
void
camera_tree_item_popup_create (GtkTreeItem* item)
{
	Camera*			camera = gtk_object_get_data (GTK_OBJECT (item), "camera");
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

