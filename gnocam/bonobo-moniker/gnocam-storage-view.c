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
#include <gal/e-table/e-tree-simple.h>
#include <gal/e-table/e-cell-tree.h>
#include <gal/e-table/e-cell-text.h>

#include "e-shell-constants.h"

#define ETABLE_SPEC "\
<ETableSpecification no-headers=\"true\" selection-mode=\"single\" cursor-mode=\"line\" draw-grid=\"true\" horizontal-scrolling=\"true\"> 		\
  <ETableColumn model_col=\"0\" _title=\"Folder\" expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"render_tree\" compare=\"string\"/> 	\
  <ETableState>																		\
    <column source=\"0\"/>																\
    <grouping/>																		\
  </ETableState>																	\
</ETableSpecification>																	\
"
#define PARENT_TYPE E_TABLE_TYPE
static ETableClass* parent_class = NULL;

struct _GnoCamStorageViewPrivate {
	Bonobo_Storage	storage;
	ETreeModel*	etree;
	ETreePath*	root_node;
};

typedef struct {
	gchar*		path;
	gboolean 	directory;
} NodeData;

enum {
        DIRECTORY_SELECTED,
        FILE_SELECTED,
        DND_ACTION,
        LAST_SIGNAL
};

static unsigned int signals[LAST_SIGNAL] = { 0 };

/**************/
/* Prototypes */
/**************/

static GdkPixbuf* scale_pixbuf (GdkPixbuf* pixbuf);

/*******************************/
/* Custom marshalling function */
/*******************************/

typedef void (* GtkSignal_NONE__GDKDRAGCONTEXT_STRING_STRING_STRING) (GtkObject* object, GdkDragContext* action, const char*, const char*, const char*);

static void
marshal_NONE__GDKDRAGCONTEXT_STRING_STRING_STRING (GtkObject* object, GtkSignalFunc func, void* func_data, GtkArg* args)
{
        GtkSignal_NONE__GDKDRAGCONTEXT_STRING_STRING_STRING rfunc;

        rfunc = (GtkSignal_NONE__GDKDRAGCONTEXT_STRING_STRING_STRING) func;
        (*rfunc) (object,
                   GTK_VALUE_POINTER (args[0]),
                   GTK_VALUE_STRING (args[1]),
                   GTK_VALUE_STRING (args[2]),
                   GTK_VALUE_STRING (args[3]));
}

/*******/
/* D&D */
/*******/

enum _DndTargetType {
        DND_TARGET_TYPE_URI_LIST,
        DND_TARGET_TYPE_E_SHORTCUT
};
typedef enum _DndTargetType DndTargetType;

#define URI_LIST_TYPE   "text/uri-list"
#define E_SHORTCUT_TYPE "E-SHORTCUT"

static GtkTargetEntry source_drag_types [] = {
        { URI_LIST_TYPE, 0, DND_TARGET_TYPE_URI_LIST },
        { E_SHORTCUT_TYPE, 0, DND_TARGET_TYPE_E_SHORTCUT }
};
static const int num_source_drag_types = sizeof (source_drag_types) / sizeof (source_drag_types[0]);

static GtkTargetEntry destination_drag_types [] = {
        { URI_LIST_TYPE, 0, DND_TARGET_TYPE_URI_LIST },
        { E_SHORTCUT_TYPE, 0, DND_TARGET_TYPE_E_SHORTCUT }
};
static const int num_destination_drag_types = sizeof (destination_drag_types) / sizeof (destination_drag_types[0]);

static GtkTargetList *target_list;

/*****************/
/* E-Table model */
/*****************/

static int
etree_col_count (ETableModel* etable, void* model_data)
{
        return (2);
}

static void*
etree_duplicate_value (ETableModel*etable, int col, const void* value, void* model_data)
{
	NodeData*	data;

	g_warning ("etree_duplicate_value");

	data = g_new (NodeData, 1);

	data->path = g_strdup (((NodeData*) value)->path);
	data->directory = ((NodeData*) value)->directory;

	return (data);
}

static void
etree_free_value (ETableModel* etable, int col, void* value, void* model_data)
{
	NodeData*	data;

	g_warning ("etree_free_value");

	data = value;
	g_free (data->path);
	g_free (data);
}

static void*
etree_initialize_value (ETableModel* etable, int col, void* model_data)
{
	g_warning ("etree_initialize_value");
	
	return (NULL);
}

static gboolean
etree_value_is_empty (ETableModel* etable, int col, const void* value, void* model_data)
{
        if (col == 0)
                return !(value && *(char*) value);
        else
                return !value;
}

static char *
etree_value_to_string (ETableModel* etable, int col, const void* value, void* model_data)
{
        if (col == 0)
                return (g_strdup (value));
        else
                return (g_strdup (value ? "Yes" : "No"));
}

/****************/
/* E-Tree model */
/****************/

static GdkPixbuf*
etree_icon_at (ETreeModel* etree, ETreePath* node, void* model_data)
{
	NodeData*	data;

	data = (NodeData*) e_tree_model_node_get_data (etree, node);

	//FIXME: Do we have to distribute our own pixmaps?

	/* Directory? */
	if (data->directory) {
		if (!g_file_exists ("/usr/share/pixmaps/gnome-folder.png")) return (NULL);
		return (scale_pixbuf (gdk_pixbuf_new_from_file ("/usr/share/pixmaps/gnome-folder.png")));
	} 

	/* File? */
	if (!g_file_exists ("/usr/share/pixmaps/gnome-file-h.png")) return (NULL);
	return (scale_pixbuf (gdk_pixbuf_new_from_file ("/usr/share/pixmaps/gnome-file-h.png")));
}

static void*
etree_value_at (ETreeModel* etree, ETreePath* node, int col, void* model_data)
{
	NodeData*	data;

	data = (NodeData*) e_tree_model_node_get_data (etree, node);

	/* If root folder, return path. */
	if (!strcmp (data->path, "/")) return (data->path);

	return (g_basename (data->path));
}

static void
etree_set_value_at (ETreeModel* etree, ETreePath* node, int col, const void* val, void* model_data)
{
	g_warning ("etree_set_value_at");
	
	return;
}

static gboolean
etree_is_editable (ETreeModel* etree, ETreePath* path, int col, void* model_data)
{
        return FALSE;
}

static int
treepath_compare (ETreeModel* etree, ETreePath* node1, ETreePath* node2)
{
        NodeData*	data1;
	NodeData*	data2;

        data1 = e_tree_model_node_get_data (etree, node1);
        data2 = e_tree_model_node_get_data (etree, node2);

	return (strcasecmp (data1->path, data2->path));
}

/*******************/
/* E-Table methods */
/*******************/

static void
cursor_change (ETable* etable, int row)
{
	GnoCamStorageView*	storage_view;
	ETreePath*		node;
	NodeData*		data;

	storage_view = GNOCAM_STORAGE_VIEW (etable);
	node = e_tree_model_node_at_row (storage_view->priv->etree, row);

	data = (NodeData*) e_tree_model_node_get_data (storage_view->priv->etree, node);

	if (data->directory) 
		gtk_signal_emit (GTK_OBJECT (storage_view), signals [DIRECTORY_SELECTED], data->path);
	else 
		gtk_signal_emit (GTK_OBJECT (storage_view), signals [FILE_SELECTED], data->path);
}

static void
table_drag_begin (ETable* etable, int row, int col, GdkDragContext* context)
{
	g_warning ("Implement!");
}

static void
table_drag_data_get (ETable* etable, int drag_row, int drag_col, GdkDragContext* context, GtkSelectionData* selection_data, unsigned int info, guint32 time)
{
	g_warning ("Implement!");
}

static gboolean
table_drag_motion (ETable* table, int row, int col, GdkDragContext* context, int x, int y, unsigned int time)
{
	g_warning ("Implement!");
	return (TRUE);
}

static gboolean
table_drag_drop (ETable* etable, int row, int col, GdkDragContext* context, int x, int y, unsigned int time)
{
	g_warning ("Implement!");
	return (FALSE);
}

static void
table_drag_data_received (ETable* etable, int row, int col, GdkDragContext* context, int x, int y, 
	GtkSelectionData* selection_data, unsigned int info, unsigned int time)
{
	g_warning ("Implement!");
}

/********************/
/* Helper functions */
/********************/

static GdkPixbuf*
scale_pixbuf (GdkPixbuf* pixbuf)
{
	GdkPixbuf*	pixbuf_scaled;

	pixbuf_scaled = gdk_pixbuf_new (
		gdk_pixbuf_get_colorspace (pixbuf), gdk_pixbuf_get_has_alpha (pixbuf), gdk_pixbuf_get_bits_per_sample (pixbuf), 
		E_SHELL_MINI_ICON_SIZE, E_SHELL_MINI_ICON_SIZE);
	gdk_pixbuf_scale (pixbuf, pixbuf_scaled, 0, 0, E_SHELL_MINI_ICON_SIZE, E_SHELL_MINI_ICON_SIZE, 0.0, 0.0, 
		(double) E_SHELL_MINI_ICON_SIZE / gdk_pixbuf_get_width (pixbuf), 
		(double) E_SHELL_MINI_ICON_SIZE / gdk_pixbuf_get_height (pixbuf), GDK_INTERP_HYPER);
	return (pixbuf_scaled);
}

static void
insert_folders_and_files (GnoCamStorageView* storage_view, ETreePath* parent, const gchar* path)
{
	Bonobo_Storage_DirectoryList*	list;
	CORBA_Environment		ev;
	gint				i;

	g_warning ("insert_folders_and_files (?, ?, %s)", path);

	CORBA_exception_init (&ev);
	list = Bonobo_Storage_listContents (storage_view->priv->storage, path, Bonobo_FIELD_TYPE, &ev);
	if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		return;
	}
	CORBA_exception_free (&ev);

	for (i = 0; i < list->_length; i++) {
		ETreePath*	node;
		gchar*		tmp;
		NodeData*	data;
		
		tmp = g_strconcat ("/", list->_buffer [i].name, NULL);
		node = e_tree_model_node_insert_id (storage_view->priv->etree, parent, -1, tmp, tmp);
		g_free (tmp);

		/* Store some data in this node */
		data = g_new (NodeData, 1);
		if (!strcmp (path, "/")) 
			data->path = g_strconcat (path, list->_buffer [i].name, NULL);
		else
			data->path = g_strconcat (path, G_DIR_SEPARATOR, list->_buffer [i].name, NULL);
		data->directory = (list->_buffer [i].type == Bonobo_STORAGE_TYPE_DIRECTORY);
		e_tree_model_node_set_data (storage_view->priv->etree, node, (gpointer) data);
		
		e_tree_model_node_set_expanded (storage_view->priv->etree, parent, TRUE);
		e_tree_model_node_set_compare_function (storage_view->priv->etree, node, treepath_compare);

		/* If this is a directory, fill it */
		if (data->directory) 
			insert_folders_and_files (storage_view, node, data->path);
	}
	
}

/*******************/
/* GtkObject stuff */
/*******************/

static void
destroy (GtkObject* object)
{
	GnoCamStorageView*	storage_view;

	storage_view = GNOCAM_STORAGE_VIEW (object);
	g_free (storage_view->priv);
	//FIXME: Free NodeData in all nodes!

	(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
class_init (GnoCamStorageViewClass* klass)
{
	GtkObjectClass*	object_class;
	ETableClass*	etable_class;

	parent_class = gtk_type_class (e_table_get_type ());

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = destroy;

	etable_class = E_TABLE_CLASS (klass);
	etable_class->cursor_change 		= cursor_change;
	etable_class->table_drag_begin		= table_drag_begin;
	etable_class->table_drag_data_get	= table_drag_data_get;
	etable_class->table_drag_motion		= table_drag_motion;
	etable_class->table_drag_drop		= table_drag_drop;
	etable_class->table_drag_data_received	= table_drag_data_received;

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

	signals[DND_ACTION] = gtk_signal_new ("dnd_action",
                           		GTK_RUN_FIRST,
                           		object_class->type,
                           		GTK_SIGNAL_OFFSET (GnoCamStorageViewClass, dnd_action),
                           		marshal_NONE__GDKDRAGCONTEXT_STRING_STRING_STRING,
                           		GTK_TYPE_NONE, 4,
                           		GTK_TYPE_GDK_DRAG_CONTEXT,
                           		GTK_TYPE_STRING,
                           		GTK_TYPE_STRING,
                           		GTK_TYPE_STRING);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	target_list = gtk_target_list_new (source_drag_types, num_source_drag_types);
}

static void
init (GnoCamStorageView* storage_view)
{
	storage_view->priv = g_new (GnoCamStorageViewPrivate, 1);
	storage_view->priv->etree = NULL;
	storage_view->priv->storage = CORBA_OBJECT_NIL;
}

GtkWidget*
gnocam_storage_view_new (Bonobo_Storage storage)
{
	GnoCamStorageView*	new;
	ETableExtras*		extras;
	ECell*			cell;
	NodeData*		data;

	new = gtk_type_new (gnocam_storage_view_get_type ());
	new->priv->storage = storage;

	/* Create the model */
	new->priv->etree = e_tree_simple_new (etree_col_count,
                                               etree_duplicate_value,
                                               etree_free_value,
                                               etree_initialize_value,
                                               etree_value_is_empty,
                                               etree_value_to_string,
                                               etree_icon_at,
                                               etree_value_at,
                                               etree_set_value_at,
                                               etree_is_editable,
                                               new);

	/* Insert the root node */
	data = g_new (NodeData, 1);
	data->path = g_strdup ("/");
	data->directory = TRUE;
	new->priv->root_node = e_tree_model_node_insert (new->priv->etree, NULL, -1, "/");
	e_tree_model_node_set_data (new->priv->etree, new->priv->root_node, data);
	
	/* Create extras */
	extras = e_table_extras_new ();
	cell = e_cell_text_new (NULL, GTK_JUSTIFY_LEFT);
	e_table_extras_add_cell (extras, "render_tree", e_cell_tree_new (NULL, NULL, TRUE, cell));

	/* Construct the table */
        e_table_construct (E_TABLE (new), E_TABLE_MODEL (new->priv->etree), extras, ETABLE_SPEC, NULL);
        gtk_object_unref (GTK_OBJECT (extras));

	/* D&D */
        e_table_drag_source_set (E_TABLE (new), GDK_BUTTON1_MASK, source_drag_types, num_source_drag_types, GDK_ACTION_MOVE | GDK_ACTION_COPY); 
        e_table_drag_dest_set (E_TABLE (new), GTK_DEST_DEFAULT_ALL, source_drag_types, num_source_drag_types, GDK_ACTION_MOVE | GDK_ACTION_COPY);

	/* Populate the tree */
	insert_folders_and_files (new, new->priv->root_node, "/");

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_storage_view, "GnoCamStorageView", GnoCamStorageView, class_init, init, PARENT_TYPE)

