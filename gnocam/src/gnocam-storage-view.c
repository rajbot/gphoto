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

#include <gphoto2.h>

#include "gnocam-storage-view.h"

#include <gnome.h>
#include <gal/util/e-util.h>
#include <gal/e-table/e-tree-simple.h>
#include <gal/e-table/e-cell-tree.h>
#include <gal/e-table/e-cell-text.h>

//#include "e-shell-constants.h"
#include "utils.h"

#define ETABLE_SPEC 																		\
"<ETableSpecification no-headers=\"true\" selection-mode=\"single\" cursor-mode=\"line\" draw-grid=\"true\" horizontal-scrolling=\"true\">" 			\
"  <ETableColumn model_col=\"0\" _title=\"Folder\" expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"render_tree\" compare=\"string\"/>" 	\
"  <ETableState>"																		\
"    <column source=\"0\"/>"																	\
"    <grouping/>"																		\
"  </ETableState>"																		\
"</ETableSpecification>"

#define PARENT_TYPE E_TABLE_TYPE
static ETableClass* parent_class = NULL;

struct _GnoCamStorageViewPrivate {

	BonoboStorage*	storage;

	ETreeModel*	model;
};

typedef struct {
	gchar*		path;
	gboolean 	directory;
	ETreePath*	placeholder;
} NodeValue;

enum {
        DIRECTORY_SELECTED,
        FILE_SELECTED,
        DND_ACTION,
        LAST_SIGNAL
};

static unsigned int signals[LAST_SIGNAL] = { 0 };

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

static gint
col_count (ETableModel* model, gpointer user_data)
{
	return (1);
}

static void
free_value (ETableModel* model, gint col, gpointer value, gpointer user_data)
{
	g_free (((NodeValue*) value)->path);
	g_free (value);
}

/****************/
/* E-Tree model */
/****************/

static GdkPixbuf*
etree_icon_at (ETreeModel* model, ETreePath* node, gpointer user_data)
{
	NodeValue*	value;

	value = (NodeValue*) e_tree_model_node_get_data (model, node);
	if (!value) return (NULL);

	//FIXME: Do we have to distribute our own pixmaps?

	/* Directory? */
	if (value->directory) return (util_pixbuf_folder ());

	/* File? */
	return (util_pixbuf_file ());
}

static void*
etree_value_at (ETreeModel* model, ETreePath* node, int col, gpointer user_data)
{
	NodeValue*	value;

	value = (NodeValue*) e_tree_model_node_get_data (model, node);
	if (!value) return (NULL);

	/* If root folder, return path. */
	if (!strcmp (value->path, "/")) return (value->path);

	return (g_basename (value->path));
}

static gboolean
etree_is_editable (ETreeModel* model, ETreePath* path, int col, gpointer user_data)
{
        return FALSE;
}

static int
treepath_compare (ETreeModel* model, ETreePath* node1, ETreePath* node2)
{
	NodeValue*	value1;
	NodeValue*	value2;

	value1 = e_tree_model_node_get_data (model, node1);
	if (!value1) return (0);
	value2 = e_tree_model_node_get_data (model, node2);
	if (!value2) return (0);

	return (strcasecmp (value1->path, value2->path));
}

/******************/
/* E-Tree signals */
/******************/

static void
on_node_expanded (ETreeModel* model, ETreePath* parent, gboolean* allow_expanded, gpointer user_data)
{
	GnoCamStorageView*		storage_view;
	Bonobo_Storage_DirectoryList*   list;
	CORBA_Environment               ev;
	gint                            i;
	NodeValue*			value;

	storage_view = GNOCAM_STORAGE_VIEW (user_data);

	value = e_tree_model_node_get_data (model, parent);
	g_return_if_fail (value);

	/* Did we already populate this part of the tree? */
	if (!value->placeholder) return;

	/* Get the list of contents */
	CORBA_exception_init (&ev);
	if (!strcmp (value->path, "/"))
		list = Bonobo_Storage_listContents (BONOBO_OBJREF (storage_view->priv->storage), "", Bonobo_FIELD_TYPE, &ev);
	else
		list = Bonobo_Storage_listContents (BONOBO_OBJREF (storage_view->priv->storage), value->path + 1, Bonobo_FIELD_TYPE, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning (_("Could not get list of contents for '%s': %s!"), value->path, bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		*allow_expanded = FALSE;
		return;
	}
	CORBA_exception_free (&ev);
	*allow_expanded = TRUE;

	/* Remove placeholder */
	if (value->placeholder) {
		e_tree_model_node_remove (model, value->placeholder);
		value->placeholder = NULL;
	}

	for (i = 0; i < list->_length; i++) {
                ETreePath*      node;
                NodeValue*      new_value;

                /* Insert the node */
		new_value = g_new (NodeValue, 1);
                if (!strcmp (value->path, "/"))
                        new_value->path = g_strconcat (value->path, list->_buffer [i].name, NULL);
                else
                        new_value->path = g_strdup_printf ("%s/%s", value->path, list->_buffer [i].name);
		new_value->directory = (list->_buffer [i].type == Bonobo_STORAGE_TYPE_DIRECTORY);
                node = e_tree_model_node_insert (model, parent, i, new_value);

                e_tree_model_node_set_expanded (model, node, FALSE);
		e_tree_model_node_set_compare_function (model, node, treepath_compare);

		/* If directory, insert a placeholder */
		if (new_value->directory)
			new_value->placeholder = e_tree_model_node_insert (model, node, 0, NULL);
		else 
			new_value->placeholder = NULL;
	}

	CORBA_free (list);
}

/*******************/
/* E-Table methods */
/*******************/

static void
cursor_change (ETable* etable, int row)
{
	GnoCamStorageView*	storage_view;
	ETreePath*		node;
	NodeValue*		value;

	storage_view = GNOCAM_STORAGE_VIEW (etable);
	node = e_tree_model_node_at_row (storage_view->priv->model, row);

	value = (NodeValue*) e_tree_model_node_get_data (storage_view->priv->model, node);

	if (value->directory) 
		gtk_signal_emit (GTK_OBJECT (storage_view), signals [DIRECTORY_SELECTED], value->path);
	else 
		gtk_signal_emit (GTK_OBJECT (storage_view), signals [FILE_SELECTED], value->path);
}

/*******************/
/* GtkObject stuff */
/*******************/

static void
gnocam_storage_view_destroy (GtkObject* object)
{
	GnoCamStorageView*	storage_view;

	storage_view = GNOCAM_STORAGE_VIEW (object);

	g_free (storage_view->priv);
	storage_view->priv = NULL;

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_storage_view_class_init (GnoCamStorageViewClass* klass)
{
	GtkObjectClass*	object_class;
	ETableClass*	etable_class;

	parent_class = gtk_type_class (PARENT_TYPE);

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = gnocam_storage_view_destroy;

	etable_class = E_TABLE_CLASS (klass);
	etable_class->cursor_change 		= cursor_change;

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
gnocam_storage_view_init (GnoCamStorageView* storage_view)
{
	storage_view->priv = g_new0 (GnoCamStorageViewPrivate, 1);
}

GtkWidget*
gnocam_storage_view_new (BonoboStorage* storage)
{
	GnoCamStorageView*	new;
	ETableExtras*		extras;
	ETreePath*		root;
	ECell*			cell;
	NodeValue*		value;

	new = gtk_type_new (GNOCAM_TYPE_STORAGE_VIEW);
	new->priv->storage = storage;

	/* Create the model */
	new->priv->model = e_tree_simple_new (col_count, NULL, free_value, NULL, NULL, NULL, etree_icon_at, etree_value_at, NULL, etree_is_editable, new);
	e_tree_model_root_node_set_visible (new->priv->model, TRUE);
	gtk_signal_connect (GTK_OBJECT (new->priv->model), "node_expanded", GTK_SIGNAL_FUNC (on_node_expanded), new);

	/* Create extras */
	extras = e_table_extras_new ();
	cell = e_cell_text_new (NULL, GTK_JUSTIFY_LEFT);
	e_table_extras_add_cell (extras, "render_tree", e_cell_tree_new (NULL, NULL, TRUE, cell));

	/* Construct the table */
        e_table_construct (E_TABLE (new), E_TABLE_MODEL (new->priv->model), extras, ETABLE_SPEC, NULL);
        gtk_object_unref (GTK_OBJECT (extras));

        /* Insert the root node */
	value = g_new (NodeValue, 1);
	value->path = g_strdup ("/");
	value->directory = TRUE;
	root = e_tree_model_node_insert (new->priv->model, NULL, -1, value);
	e_tree_model_node_set_expanded (new->priv->model, root, FALSE);

	/* Insert placeholder */
	value->placeholder = e_tree_model_node_insert (new->priv->model, root, 0, NULL);

	/* D&D */
        e_table_drag_source_set (E_TABLE (new), GDK_BUTTON1_MASK, source_drag_types, num_source_drag_types, GDK_ACTION_MOVE | GDK_ACTION_COPY); 
        e_table_drag_dest_set (E_TABLE (new), GTK_DEST_DEFAULT_ALL, source_drag_types, num_source_drag_types, GDK_ACTION_MOVE | GDK_ACTION_COPY);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_storage_view, "GnoCamStorageView", GnoCamStorageView, gnocam_storage_view_class_init, gnocam_storage_view_init, PARENT_TYPE)

