/* gnocam-storage-view.c
 *
 * Adapted from:
 * e-storage-set-view.c
 *
 * Copyright (C) 2000, 2001 Ximian, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Ettore Perazzoli
 * Etree-ification: Chris Toshok
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-storage-view.h"

#include <gnome.h>
#include <gal/util/e-util.h>
#include <gal/e-table/e-tree-memory.h>
#include <gal/e-table/e-tree-memory-callbacks.h>
#include <gal/e-table/e-cell-tree.h>
#include <gal/e-table/e-cell-text.h>

#include "utils.h"

#define ETABLE_SPEC 																		\
"<ETableSpecification no-headers=\"true\" selection-mode=\"single\" cursor-mode=\"line\" draw-grid=\"true\" horizontal-scrolling=\"true\">" 			\
"  <ETableColumn model_col=\"0\" _title=\"Folder\" expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"render_tree\" compare=\"string\"/>" 	\
"  <ETableState>"																		\
"    <column source=\"0\"/>"																	\
"    <grouping/>"																		\
"  </ETableState>"																		\
"</ETableSpecification>"

#define PARENT_TYPE E_TREE_SCROLLED_TYPE
static ETreeScrolledClass* parent_class = NULL;

struct _GnoCamStorageViewPrivate {

	Camera*		camera;

	GConfClient*	client;

	ETree*		tree;
	ETreeModel*	model;

	GHashTable*	hash_table;
};

typedef struct {
	gchar*		path;
	gboolean 	directory;
	Bonobo_Storage	storage;
	gboolean	populated;
} NodeValue;

enum {
        DIRECTORY_SELECTED,
        FILE_SELECTED,
        LAST_SIGNAL
};

static unsigned int signals[LAST_SIGNAL] = { 0 };

/*******************/
/* Internally used */
/*******************/

static void
populate_node (GnoCamStorageView *storage_view, ETreePath node)
{
	CORBA_Environment		 ev;
	Bonobo_Storage_DirectoryList	*list;
	NodeValue			*value;
	gint				 i;

	value = (NodeValue*) e_tree_memory_node_get_data (E_TREE_MEMORY (storage_view->priv->model), node);
	if (!value->directory) return;
	if (!value->storage) return;

	value->populated = TRUE;

	CORBA_exception_init (&ev);

        /* Get the list of contents */
	list = Bonobo_Storage_listContents (value->storage, "", Bonobo_FIELD_TYPE, &ev);
        if (BONOBO_EX (&ev)) {
                g_warning (_("Could not get list of contents for '%s': %s!"), value->path, bonobo_exception_get_text (&ev));
                CORBA_exception_free (&ev);
                return;
        }

        CORBA_exception_free (&ev);

        for (i = 0; i < list->_length; i++) {
		ETreePath	new_node;
                NodeValue*      new_value;

		/* Insert the new node */
		new_value = g_new (NodeValue, 1);
		if (!strcmp (value->path, "/"))
                        new_value->path = g_strconcat (value->path, list->_buffer [i].name, NULL);
                else
                        new_value->path = g_strdup_printf ("%s/%s", value->path, list->_buffer [i].name);
                new_value->directory = (list->_buffer [i].type == Bonobo_STORAGE_TYPE_DIRECTORY);
		new_value->populated = !new_value->directory;
		new_node = e_tree_memory_node_insert (E_TREE_MEMORY (storage_view->priv->model), node, -1, new_value);
                g_hash_table_insert (storage_view->priv->hash_table, new_value->path, new_node);

		/* If directory, open the storage */
		if (new_value->directory) {
			CORBA_Environment	ev;
			Bonobo_Storage_OpenMode	mode;

			CORBA_exception_init (&ev);

			/* Get the new storage */
		        mode = Bonobo_Storage_READ;
		        if ((storage_view->priv->camera->abilities->file_operations & GP_FILE_OPERATION_PREVIEW) &&         
		            gconf_client_get_bool (storage_view->priv->client, "/apps/" PACKAGE "/preview", NULL))
		                mode |= Bonobo_Storage_COMPRESSED;
			new_value->storage = Bonobo_Storage_openStorage (value->storage, g_basename (new_value->path), mode, &ev);
		        if (BONOBO_EX (&ev)) g_warning (_("Could not open storage for '%s': %s!"), new_value->path, bonobo_exception_get_text (&ev));

	                CORBA_exception_free (&ev);

		}
        }
}

/****************/
/* E-Tree model */
/****************/

static gint
column_count (ETreeModel* model, gpointer data)
{
	return (1);
}

static void
free_value (ETreeModel* model, gint col, gpointer val, gpointer data)
{
	NodeValue*	value;

	value = (NodeValue*) val;

	g_free (value->path);
	bonobo_object_release_unref (value->storage, NULL);

	g_free (value);
}

static GdkPixbuf*
icon_at (ETreeModel* model, ETreePath node, gpointer data)
{
	NodeValue*	value;

	value = (NodeValue*) e_tree_memory_node_get_data (E_TREE_MEMORY (model), node);
	if (!value) return (NULL);

	/* Directory? */
	if (value->directory) return (util_pixbuf_folder ());

	/* File? */
	return (util_pixbuf_file ());
}

static void*
value_at (ETreeModel* model, ETreePath node, gint col, gpointer data)
{
	GnoCamStorageView*	storage_view;
	NodeValue*		value;

	storage_view = GNOCAM_STORAGE_VIEW (data);

	value = (NodeValue*) e_tree_memory_node_get_data (E_TREE_MEMORY (model), node);

	/* Have we already populate this node? */
	if (!value->populated)
		populate_node (storage_view, node);

	/* If root folder, return path. */
	if (!strcmp (value->path, "/")) return (value->path);

	return (g_basename (value->path));
}

static void
set_value_at (ETreeModel *model, ETreePath node, gint col, const void *value, gpointer data)
{
	/* Nothing in here yet */
}

static gboolean
is_editable (ETreeModel *model, ETreePath path, int col, gpointer data)
{
        return (FALSE);
}

static gboolean
value_is_empty (ETreeModel *model, gint col, const void *value, gpointer data)
{
	return (!value);
}

static void*
duplicate_value (ETreeModel *model, gint col, const void *value, gpointer data)
{
	/* Nothing in here yet */

	return (NULL);
}

static void*
initialize_value (ETreeModel *model, gint col, gpointer data)
{
	/* Nothing in here yet */

	return (NULL);
}

static gchar*
value_to_string (ETreeModel* mode, gint col, const void* value, gpointer data)
{
	return (g_strdup (((NodeValue*)value)->path));
}

/*************/
/* Callbacks */
/*************/

static void
on_folder_updated (GnoCamCamera* camera, const gchar* path, GnoCamStorageView* storage_view)
{
	ETreePath*		node;
	ETreePath*		child;
	gboolean		expanded;

	node = g_hash_table_lookup (storage_view->priv->hash_table, path);
	if (!node) return;

	expanded = e_tree_node_is_expanded (storage_view->priv->tree, node);

	e_tree_node_set_expanded (E_TREE (storage_view), node, FALSE);
	while (TRUE) {
		child = e_tree_model_node_get_first_child (storage_view->priv->model, node);
		if (!child) break;
		e_tree_memory_node_remove (E_TREE_MEMORY (storage_view->priv->model), child);
	}

	populate_node (storage_view, node);
	if (expanded) e_tree_node_set_expanded (storage_view->priv->tree, node, TRUE);
}


static void
on_cursor_change (ETree* etree, int row, ETreePath path, GnoCamStorageView* storage_view)
{
	ETreePath*	node;
	NodeValue*	value;

	node = e_tree_node_at_row (storage_view->priv->tree, row);
	value = (NodeValue*) e_tree_memory_node_get_data (E_TREE_MEMORY (storage_view->priv->model), node);

	if (value->directory) gtk_signal_emit (GTK_OBJECT (storage_view), signals [DIRECTORY_SELECTED], value->path);
	else gtk_signal_emit (GTK_OBJECT (storage_view), signals [FILE_SELECTED], value->path);
}


/*******************/
/* GtkObject stuff */
/*******************/

static void
gnocam_storage_view_destroy (GtkObject* object)
{
	GnoCamStorageView*	storage_view;

	storage_view = GNOCAM_STORAGE_VIEW (object);

	gp_camera_unref (storage_view->priv->camera);
	gtk_object_unref (GTK_OBJECT (storage_view->priv->client));

	g_hash_table_destroy (storage_view->priv->hash_table);

	g_free (storage_view->priv);
	storage_view->priv = NULL;

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_storage_view_class_init (GnoCamStorageViewClass* klass)
{
	GtkObjectClass*		object_class;

	parent_class = gtk_type_class (PARENT_TYPE);

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = gnocam_storage_view_destroy;

	signals[DIRECTORY_SELECTED] = gtk_signal_new ("directory_selected",
	                                GTK_RUN_FIRST,
        	                        object_class->type,
                	                GTK_SIGNAL_OFFSET (GnoCamStorageViewClass, directory_selected),
                        	        gtk_marshal_NONE__STRING,
	                                GTK_TYPE_NONE, 1,
        	                        GTK_TYPE_STRING);

	signals[FILE_SELECTED] = gtk_signal_new ("file_selected",
                	                GTK_RUN_FIRST,
                        	        object_class->type,
                                	GTK_SIGNAL_OFFSET (GnoCamStorageViewClass, file_selected), 
					gtk_marshal_NONE__STRING,
        	                        GTK_TYPE_NONE, 1,
                	                GTK_TYPE_STRING);
	
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
}

static void
gnocam_storage_view_init (GnoCamStorageView* storage_view)
{
	storage_view->priv = g_new0 (GnoCamStorageViewPrivate, 1);
}

GtkWidget*
gnocam_storage_view_new (GnoCamCamera* camera)
{
	GnoCamStorageView*		new;
	ETableExtras*			extras;
	ETreePath*			root;
	ECell*				cell;
	NodeValue*			value;

	new = GNOCAM_STORAGE_VIEW (gtk_widget_new (GNOCAM_TYPE_STORAGE_VIEW, "hadjustment", NULL, "vadjustment", NULL, NULL));
	new->priv->client = gnocam_camera_get_client (camera);
	new->priv->camera = gnocam_camera_get_camera (camera);
	new->priv->hash_table = g_hash_table_new (g_str_hash, g_str_equal);

	/* Create the model */
	new->priv->model = e_tree_memory_callbacks_new (icon_at, column_count, NULL, NULL, value_at, set_value_at, is_editable, duplicate_value, free_value, 
							initialize_value, value_is_empty, value_to_string, new);

	/* Create extras */
	extras = e_table_extras_new ();
	cell = e_cell_text_new (NULL, GTK_JUSTIFY_LEFT);
	e_table_extras_add_cell (extras, "render_tree", e_cell_tree_new (NULL, NULL, TRUE, cell));

	/* Construct the scrolled tree */
	e_tree_scrolled_construct (E_TREE_SCROLLED (new), new->priv->model, extras, ETABLE_SPEC, NULL);
	gtk_object_unref (GTK_OBJECT (extras));

	new->priv->tree = e_tree_scrolled_get_tree (E_TREE_SCROLLED (new));

	/* Set up the root node */
	value = g_new (NodeValue, 1);
	value->path = g_strdup ("/");
        value->directory = TRUE;
	value->populated = FALSE;
        value->storage = gnocam_camera_get_storage (camera);

        /* Insert the root node */
	root = e_tree_memory_node_insert (E_TREE_MEMORY (new->priv->model), NULL, -1, value);
	e_tree_node_set_expanded (new->priv->tree, root, FALSE);
	g_hash_table_insert (new->priv->hash_table, value->path, root);

	gtk_signal_connect (GTK_OBJECT (camera), "folder_updated", GTK_SIGNAL_FUNC (on_folder_updated), new);

	/* Configure the tree */
	e_tree_root_node_set_visible (new->priv->tree, TRUE);
	gtk_signal_connect (GTK_OBJECT (new->priv->tree), "cursor_change", GTK_SIGNAL_FUNC (on_cursor_change), new);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_storage_view, "GnoCamStorageView", GnoCamStorageView, gnocam_storage_view_class_init, gnocam_storage_view_init, PARENT_TYPE)

