/**
 * This file should be called utils.c or so.
 * I'll change the name some day.
 * 
 * Should this file be split up? 
 */

#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include <parser.h>
#include "gphoto-extensions.h"
#include "callbacks.h"
#include "gnocam.h"
#include "cameras.h"
#include "information.h"
#include "file-operations.h"

/**********************/
/* External Variables */
/**********************/

extern GladeXML*	xml;
extern GConfClient*	client;

/**************/
/* Prototypes */
/**************/

void on_drag_data_received                      (GtkWidget* widget, GdkDragContext* context, gint x, gint y, GtkSelectionData* selection_data, guint info, guint time);
void on_camera_tree_file_drag_data_get          (GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data, guint info, guint time, gpointer data);
void on_camera_tree_folder_drag_data_get        (GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data, guint info, guint time, gpointer data);

/*************/
/* Callbacks */
/*************/

void
on_camera_tree_file_drag_data_get (GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data, guint info, guint time, gpointer data)
{
	gchar*		filename;
	CameraFile*	file;

	if (!(file = gtk_object_get_data (GTK_OBJECT (widget), "file"))) g_assert ((file = gtk_object_get_data (GTK_OBJECT (widget), "preview")) != NULL);
	filename = g_strdup_printf ("file:/tmp/%s", file->name);
	camera_file_save (file, filename);
	gtk_selection_data_set (selection_data, selection_data->target, 8, filename, strlen (filename));
	g_free (filename);
}

void
on_camera_tree_folder_drag_data_get (GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data, guint info, guint time, gpointer data)
{
	dialog_information (_("Not yet implemented!"));
}

void
on_drag_data_received (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint time)
{
        GList*                  filenames;
        guint                   i;
        gchar*                  path;
        Camera*                 camera;

        g_assert ((path = gtk_object_get_data (GTK_OBJECT (widget), "path")) != NULL);
        g_assert ((camera = gtk_object_get_data (GTK_OBJECT (widget), "camera")) != NULL);

        filenames = gnome_uri_list_extract_filenames (selection_data->data);
        for (i = 0; i < g_list_length (filenames); i++) {
                upload (GTK_TREE_ITEM (widget), g_list_nth_data (filenames, i));
        }
        gnome_uri_list_free_strings (filenames);
}

/*************/
/* Functions */
/*************/

void
camera_tree_folder_clean (GtkTreeItem* folder)
{
	gint		i;
	GtkTree*	tree;

	g_assert (folder != NULL);
	g_assert ((tree = GTK_TREE (folder->subtree)) != NULL);

	/* Delete all items of tree. */
	for (i = g_list_length (tree->children) - 1; i >= 0; i--) camera_tree_item_remove (GTK_TREE_ITEM (g_list_nth_data (tree->children, i)));
}

void 
camera_tree_folder_populate (GtkTreeItem* folder)
{
        CameraList              folder_list, file_list;
        CameraListEntry*        folder_list_entry;
        CameraListEntry*        file_list_entry;
        Camera*                 camera;
        gchar*                  path;
        gchar*                  new_path;
        gint                    folder_list_count, file_list_count;
        gint                    i;

        g_assert ((camera = gtk_object_get_data (GTK_OBJECT (folder), "camera")) != NULL);
        g_assert ((path = gtk_object_get_data (GTK_OBJECT (folder), "path")) != NULL);
        g_assert (folder->subtree);
        g_assert (GTK_OBJECT (folder->subtree)->ref_count > 0);

        /* Get file and folder list. */
        if (gp_camera_folder_list (camera, &folder_list, path) != GP_OK) {
                dialog_information ("Could not get folder list for folder '%s'!", path);
                return;
        }
        if (gp_camera_file_list (camera, &file_list, path) != GP_OK) {
                dialog_information ("Could not get file list for folder '%s'!", path);
                return;
        }

        /* Add folders to tree. */
        folder_list_count = gp_list_count (&folder_list);
        if (folder_list_count > 0) {
                for (i = 0; i < folder_list_count; i++) {
                        folder_list_entry = gp_list_entry (&folder_list, i);

                        /* Construct the new path. */
                        if (strcmp (path, "/") == 0) new_path = g_strdup_printf ("/%s", folder_list_entry->name);
                        else new_path = g_strdup_printf ("%s/%s", path, folder_list_entry->name);

                        /* Add the folder to the tree. */
                        camera_tree_folder_add (GTK_TREE (folder->subtree), camera, new_path);

                        /* Clean up. */
                        g_free (new_path);
                }
        }

        /* Add files to tree. */
        file_list_count = gp_list_count (&file_list);
        if (file_list_count > 0) {
                for (i = 0; i < file_list_count; i++) {
                        file_list_entry = gp_list_entry (&file_list, i);
                        camera_tree_file_add (GTK_TREE (folder->subtree), camera, path, file_list_entry->name);
                }
        }

	gtk_object_set_data (GTK_OBJECT (folder), "populated", GINT_TO_POINTER (1));
}

void
camera_tree_folder_refresh (GtkTreeItem* folder)
{
	gboolean expanded;

	expanded = folder->expanded;

	/* Clean the folder... */
	camera_tree_folder_clean (folder);

	/* ... and fill it. */
	camera_tree_folder_populate (folder);

	if (expanded) gtk_tree_item_expand (folder);
}

void
camera_tree_item_remove (GtkTreeItem* item)
{
	GladeXML*		xml_page;
	gchar*			path;
	gchar*			filename;
	GtkNotebook*		notebook;
	GtkWidget*		owner;
	GtkTree*		tree;
	Camera*			camera;
	gboolean		root;
	CameraFile*		file;

	g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);
	g_assert ((notebook = GTK_NOTEBOOK (glade_xml_get_widget (xml, "notebook_files"))) != NULL);
	g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);
	g_assert ((tree = GTK_TREE (GTK_WIDGET (item)->parent)) != NULL);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);

	/* Root folder needs special care. */
	root = ((!(filename = gtk_object_get_data (GTK_OBJECT (item), "filename"))) && (strcmp ("/", path) == 0));

	/* If item is a folder, clean it. */
	if (!filename) {
		g_assert (item->subtree);
		camera_tree_folder_clean (item);
		gtk_widget_unref (item->subtree);
	}

        /* Do we have to remove a notebook page? */
        if ((xml_page = gtk_object_get_data (GTK_OBJECT (item), "xml_page"))) page_remove (xml_page);

	/* If it's the root folder, unref the camera. */
	if (root) {
		gp_camera_unref (camera);
	}

	/* If this is the last item, we have to make sure we don't loose the 	*/
	/* tree. Therefore, keep a reference to the tree owner. 		*/
	owner = tree->tree_owner;

	/* Clean up. */
	g_free (path);
	if (filename) g_free (filename);
	if ((file = gtk_object_get_data (GTK_OBJECT (item), "file"))) gp_file_unref (file);
	if ((file = gtk_object_get_data (GTK_OBJECT (item), "preview"))) gp_file_unref (file);
        gtk_container_remove (GTK_CONTAINER (tree), GTK_WIDGET (item));

	/* Make sure the tree does not get lost. */
	if (GTK_WIDGET (tree)->parent == NULL) {

		/* This does only happen with tree items. I hope so. */
		g_assert (GTK_IS_TREE_ITEM (owner));
		gtk_tree_item_set_subtree (GTK_TREE_ITEM (owner), GTK_WIDGET (tree));
	}
}

void
camera_tree_folder_add (GtkTree* tree, Camera* camera, gchar* path)
{
	GtkWidget*	item;
	GtkWidget*	subtree;
	gboolean 	root;
	CameraList	folder_list;
	CameraList	file_list;
	gint		folder_list_count;
	gint		file_list_count;
	GtkTargetEntry 	target_table[] = {{"text/uri-list", 0, 0}};

	g_assert (camera != NULL);
	g_assert (tree != NULL);
	g_assert (path != NULL);

	/* Root folder needs special care. */
	root = (strcmp ("/", path) == 0);

	/* Create the new item. */
	if (root) item = gtk_tree_item_new_with_label (((frontend_data_t*) camera->frontend_data)->name);
	else item = gtk_tree_item_new_with_label (path);
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
	if (root) gtk_signal_connect (GTK_OBJECT (item), "button_press_event", GTK_SIGNAL_FUNC (on_tree_item_camera_button_press_event), NULL);
	else gtk_signal_connect (GTK_OBJECT (item), "button_press_event", GTK_SIGNAL_FUNC (on_tree_item_folder_button_press_event), NULL);
	gtk_signal_connect (GTK_OBJECT (item), "drag_data_get", GTK_SIGNAL_FUNC (on_camera_tree_folder_drag_data_get), NULL);

        /* Store some data. */
        gtk_object_set_data (GTK_OBJECT (item), "camera", camera);
        gtk_object_set_data (GTK_OBJECT (item), "path", g_strdup (path));
        gtk_object_set_data (GTK_OBJECT (item), "checked", GINT_TO_POINTER (1));

        /* Do we have folders? */
        if (gp_camera_folder_list (camera, &folder_list, path) != GP_OK) {
                dialog_information ("Could not get folder list for folder '%s'!", path);
                folder_list_count = 0;
        } else folder_list_count = gp_list_count (&folder_list);
        gtk_object_set_data (GTK_OBJECT (item), "folder_list_count", GINT_TO_POINTER (folder_list_count));

        /* Do we have files? */
        if (gp_camera_file_list (camera, &file_list, path) != GP_OK) {
                dialog_information ("Could not get file list for folder '%s'!", path);
                file_list_count = 0;
        } else file_list_count = gp_list_count (&file_list);
        gtk_object_set_data (GTK_OBJECT (item), "file_list_count", GINT_TO_POINTER (file_list_count));

        /* Create the subtree. Don't populate it yet. */
        subtree = gtk_tree_new ();
        gtk_widget_ref (subtree);
        gtk_widget_show (subtree);
        gtk_tree_item_set_subtree (GTK_TREE_ITEM (item), subtree);
}

void
camera_tree_file_add (GtkTree* tree, Camera* camera, gchar* path, gchar* filename)
{
	GtkWidget*	item;
	GtkTargetEntry  target_table[] = {{"text/uri-list", 0, 0}};

	g_assert (tree != NULL);
	g_assert (camera != NULL);
	g_assert (path != NULL);
	g_assert (filename != NULL);
	
	/* Add the file to the tree. */
        item = gtk_tree_item_new_with_label (filename);
        gtk_widget_show (item);
        gtk_tree_append (tree, item);

	/* Drag and Drop */
	gtk_drag_source_set (item, GDK_BUTTON1_MASK | GDK_BUTTON3_MASK, target_table, 1, GDK_ACTION_COPY);

        /* Store some data. */
        gtk_object_set_data (GTK_OBJECT (item), "camera", camera);
        gtk_object_set_data (GTK_OBJECT (item), "path", g_strdup (path));
        gtk_object_set_data (GTK_OBJECT (item), "filename", g_strdup (filename));

        /* Connect the signals. */
        gtk_signal_connect (GTK_OBJECT (item), "select", GTK_SIGNAL_FUNC (on_tree_item_select), NULL);
        gtk_signal_connect (GTK_OBJECT (item), "deselect", GTK_SIGNAL_FUNC (on_tree_item_deselect), NULL);
        gtk_signal_connect (GTK_OBJECT (item), "button_press_event", GTK_SIGNAL_FUNC (on_tree_item_file_button_press_event), NULL);
	gtk_signal_connect (GTK_OBJECT (item), "drag_data_get", GTK_SIGNAL_FUNC (on_camera_tree_file_drag_data_get), NULL);
}

void
pixmap_set (GtkPixmap* pm, CameraFile* file)
{
        GdkPixbuf*              pixbuf;
        GdkPixbufLoader*        loader;
        GdkPixmap*              pixmap;
        GdkBitmap*              bitmap;
	gfloat*			magnification;

	g_assert (pm != NULL);
	g_return_if_fail (pm != NULL);
	g_assert ((loader = gdk_pixbuf_loader_new ()) != NULL);

	/* Make sure the magnification is already there. */
	if (!(magnification = gtk_object_get_data (GTK_OBJECT (pm), "magnification"))) {
		magnification = g_new (gfloat, 1);
		*magnification = 1.0;
		 gtk_object_set_data (GTK_OBJECT (pm), "magnification", magnification);
	}

	/* Create the pixmap. */
	if (gdk_pixbuf_loader_write (loader, file->data, file->size)) {
		if ((pixbuf = gtk_object_get_data (GTK_OBJECT (pm), "pixbuf")) != NULL) gdk_pixbuf_unref (pixbuf);
                gdk_pixbuf_loader_close (loader);
                g_assert ((pixbuf = gdk_pixbuf_loader_get_pixbuf (loader)) != NULL);
                gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap, 127);
                gtk_pixmap_set (pm, pixmap, bitmap);
		gtk_object_set_data (GTK_OBJECT (pm), "pixbuf", pixbuf);
        } else dialog_information (_("Could not load image '%s'!"), file->name);

	/* Scale it according to gconf's values. */
	pixmap_rescale (pm, NULL);
}

void
pixmap_rescale (GtkPixmap* pm, gfloat* magnification)
{
        GdkInterpType           interpolation = 0;
        GConfValue*             value;
        GdkPixbuf*              pixbuf;
        GdkPixmap*              pixmap;
        GdkBitmap*              bitmap;
	gfloat*			magnification_absolute;

	g_assert (pm != NULL);
	if ((pixbuf = gtk_object_get_data (GTK_OBJECT (pm), "pixbuf"))) {

		/* Magnification? */
		g_assert ((magnification_absolute = gtk_object_get_data (GTK_OBJECT (pm), "magnification")) != NULL);
		if (magnification) {
			*magnification_absolute *= *magnification;
		} else {
	                value = gconf_client_get_without_default (client, "/apps/" PACKAGE "/magnification", NULL);
	                if (value) {
	                        g_assert (value->type = GCONF_VALUE_FLOAT);
	                        *magnification_absolute = gconf_value_get_float (value);
	                }
		}
		if (!((gint) (*magnification_absolute * gdk_pixbuf_get_width (pixbuf)) * (gint) (*magnification_absolute * gdk_pixbuf_get_height (pixbuf)))) { 
			dialog_information (_("Can't scale down to zero!"));
			return;
		}

                /* Interpolation? */
                value = gconf_client_get_without_default (client, "/apps/" PACKAGE "/interpolation", NULL);
                if (value) {
                        g_assert (value->type = GCONF_VALUE_INT);
                        switch (gconf_value_get_int (value)) {
                        case 0:
                                interpolation = GDK_INTERP_NEAREST;
                                break;
                        case 1:
                                interpolation = GDK_INTERP_TILES;
                                break;
                        case 2:
                                interpolation = GDK_INTERP_BILINEAR;
                                break;
                        case 3:
                                interpolation = GDK_INTERP_HYPER;
                                break;
                        default:
                                g_assert_not_reached ();
                        }
                } else interpolation = GDK_INTERP_BILINEAR;

                gdk_pixbuf_render_pixmap_and_mask (gdk_pixbuf_scale_simple (
                        pixbuf,
                        *magnification_absolute * gdk_pixbuf_get_width (pixbuf),
                        *magnification_absolute * gdk_pixbuf_get_height (pixbuf),
                        interpolation), &pixmap, &bitmap, 127);
                gtk_pixmap_set (pm, pixmap, bitmap);
	}
}

void
camera_tree_update (GtkTree* tree, GConfValue* value)
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
        gint            i;
        gint            j;
        Camera*         camera;
	gboolean	changed;

	g_assert (tree != NULL);

        if (value) {
                g_assert (value->type == GCONF_VALUE_LIST);
                g_assert (gconf_value_get_list_type (value) == GCONF_VALUE_STRING);
                list_cameras = gconf_value_get_list (value);
        }

        /* Mark each camera in the tree as unchecked. */
        for (i = 0; i < g_list_length (tree->children); i++) gtk_object_set_data (GTK_OBJECT (g_list_nth_data (tree->children, i)), "checked", GINT_TO_POINTER (0));

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
                for (j = 0; j < g_list_length (tree->children); j++) {
			item = GTK_TREE_ITEM (g_list_nth_data (tree->children, j));
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
					j = g_list_length (tree->children) - 1;
				}
                        }
                }
                if (j == g_list_length (tree->children)) {

                        /* We don't have the camera in the tree (yet). */
                        if ((camera = gp_camera_new_by_description (atoi (id), name, model, port, atoi (speed)))) camera_tree_folder_add (tree, camera, "/");
                }
        }

        /* Delete all unchecked cameras. */
        for (j = g_list_length (tree->children) - 1; j >= 0; j--) {
                item = GTK_TREE_ITEM (g_list_nth_data (tree->children, j));
                if (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item), "checked")) == 0) camera_tree_item_remove (item);
        }
}

void
camera_tree_rescale_pixmaps (GtkTree* tree)
{
	GladeXML*	xml_page;
        GtkTreeItem*    item;
        gint            i;

	g_assert (tree != NULL);

        for (i = 0; i < g_list_length (tree->children); i++) {
                item = GTK_TREE_ITEM (g_list_nth_data (tree->children, i));
                if (item->subtree) camera_tree_rescale_pixmaps (GTK_TREE (item->subtree));
                else if (gtk_object_get_data (GTK_OBJECT (item), "filename")) {
			if ((xml_page = gtk_object_get_data (GTK_OBJECT (item), "xml_page"))) {
				pixmap_rescale (GTK_PIXMAP (glade_xml_get_widget (xml_page, "pixmap_preview")), NULL);
			}
		}
        }
} 

void
app_clean_up (void)
{
	GtkTree*		tree;
	Camera*			camera;
	frontend_data_t*	frontend_data;
	gint			i;

        g_assert ((tree = GTK_TREE (glade_xml_get_widget (xml, "tree_cameras"))) != NULL);

        for (i = g_list_length (tree->children) - 1; i >= 0; i--) {

		/* Any preview window open? */
                g_assert ((camera = gtk_object_get_data (GTK_OBJECT (g_list_nth_data (tree->children, i)), "camera")) != NULL);
                g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);
                if (frontend_data->xml_preview) gtk_widget_destroy (glade_xml_get_widget (frontend_data->xml_preview, "app_preview"));

		/* Remove the tree item. */
		camera_tree_item_remove (g_list_nth_data (tree->children, i));
	}
}

void
page_remove (GladeXML* xml_page)
{
	GtkWidget*	page;
	GtkNotebook*	notebook;
	GdkPixbuf*	pixbuf;

	g_assert (xml_page != NULL);
	g_assert ((notebook = GTK_NOTEBOOK (glade_xml_get_widget (xml, "notebook_files"))) != NULL);

	if (!(page = glade_xml_get_widget (xml_page, "table_camera"))) 
		if (!(page = glade_xml_get_widget (xml_page, "table_folder"))) 
			if (!(page = glade_xml_get_widget (xml_page, "table_file"))) 
				g_assert_not_reached ();
	if ((pixbuf = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_page, "pixmap_preview")), "pixbuf"))) gdk_pixbuf_unref (pixbuf);
        gtk_notebook_remove_page (notebook, gtk_notebook_page_num (notebook, page));
}


