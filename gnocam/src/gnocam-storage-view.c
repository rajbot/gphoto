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
	value2 = e_tree_model_node_get_data (model, node2);

	return (strcasecmp (value1->path, value2->path));
}

/******************/
/* E-Tree signals */
/******************/

static void
on_node_expanded (ETreeModel* model, ETreePath* parent, gboolean* allow_expanded)
{
	Bonobo_Storage_DirectoryList*   list;
	CORBA_Environment               ev;
	gint                            i;
	NodeValue*			value;

	value = e_tree_model_node_get_data (model, parent);
	g_return_if_fail (value);

	CORBA_exception_init (&ev);
	if (!strcmp (value->path, "/"))
		list = Bonobo_Storage_listContents (BONOBO_OBJREF (storage_view->priv->storage), "", Bonobo_FIELD_TYPE, &ev);
	else
		list = Bonobo_Storage_listContents (BONOBO_OBJREF (storage_view->priv->storage), value->path + 1, Bonobo_FIELD_TYPE, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning (_("Could not get list of contents for '%s': %s!"), value->path, bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return;
	}
	CORBA_exception_free (&ev);

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
                node = e_tree_model_node_insert (storage_view->priv->model, parent, i, new_value);

                e_tree_model_node_set_compare_function (storage_view->priv->model, node, treepath_compare);
	}

	CORBA_free (list);
}

/*******************/
/* E-Table methods */
/*******************/

static gint
right_click (ETable* etable, int row, int column, GdkEvent* event)
{
	GnoCamStorageView*	storage_view;
	ETreePath*		node;
	NodeValue*		value;
	
	storage_view = GNOCAM_STORAGE_VIEW (etable);
	node = e_tree_model_node_at_row (storage_view->priv->model, row);

	value = (NodeValue*) e_tree_model_node_get_data (storage_view->priv->model, node);

	if (value->directory) {
		g_warning ("Popup folder menu: not implemented!");
	} else {
		g_warning ("Popup file menu: not implemented!");
	}

	return (0);
}

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

static void
insert_folders_and_files (GnoCamStorageView* storage_view, ETreePath* parent, const gchar* path)
{
	Bonobo_Storage_DirectoryList*	list;
	CORBA_Environment		ev;
	gint				i;

	CORBA_exception_init (&ev);
	if (!strcmp (path, "/")) 
		list = Bonobo_Storage_listContents (BONOBO_OBJREF (storage_view->priv->storage), "", Bonobo_FIELD_TYPE, &ev);
	else
		list = Bonobo_Storage_listContents (BONOBO_OBJREF (storage_view->priv->storage), path + 1, Bonobo_FIELD_TYPE, &ev);
	if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		return;
	}
	CORBA_exception_free (&ev);

	for (i = 0; i < list->_length; i++) {
		ETreePath*	node;
		NodeValue*	value;
		
		/* Insert the node */
		value = g_new (NodeValue, 1);
		if (!strcmp (path, "/"))
			value->path = g_strconcat (path, list->_buffer [i].name, NULL);
		else
			value->path = g_strdup_printf ("%s/%s", path, list->_buffer [i].name);
		value->directory = (list->_buffer [i].type == Bonobo_STORAGE_TYPE_DIRECTORY);
		node = e_tree_model_node_insert (storage_view->priv->model, parent, i, value);
		
		e_tree_model_node_set_expanded (storage_view->priv->model, parent, TRUE);
		e_tree_model_node_set_compare_function (storage_view->priv->model, node, treepath_compare);

		//The following is in order to protect us against "Directory Browse" 
		//browsing through the whole filesystem.
		//I know, the following lines really suck...
		if (!strcmp (list->_buffer [i].name, "dev")) continue;
		if (!strcmp (list->_buffer [i].name, "bin")) continue;
		if (!strcmp (list->_buffer [i].name, "usr")) continue;
		if (!strcmp (list->_buffer [i].name, "var")) continue;
		if (!strcmp (list->_buffer [i].name, "root")) continue;
		if (!strcmp (list->_buffer [i].name, "dev-state")) continue;
		if (!strcmp (list->_buffer [i].name, "etc")) continue;
		if (!strcmp (list->_buffer [i].name, "boot")) continue;
		if (!strcmp (list->_buffer [i].name, "opt")) continue;
		if (!strcmp (list->_buffer [i].name, "mnt")) continue;
		if (!strcmp (list->_buffer [i].name, "lib")) continue;
		if (!strcmp (list->_buffer [i].name, "sbin")) continue;
		if (!strcmp (list->_buffer [i].name, "tmp")) continue;
		if (!strcmp (list->_buffer [i].name, "lost+found")) continue;
		if (!strcmp (list->_buffer [i].name, "proc")) continue;
		if (!strcmp (list->_buffer [i].name, "evolution")) continue;
		if (!strcmp (list->_buffer [i].name, "src")) continue;
		if (list->_buffer [i].name [0] == '.') continue;

		/* If this is a directory, fill it */
		if (value->directory) 
			insert_folders_and_files (storage_view, node, value->path);
	}
	
}

static gint
populate_tree (gpointer user_data)
{
	GnoCamStorageView*	storage_view;
	ETreePath*              root;
	NodeValue*		value;

	storage_view = GNOCAM_STORAGE_VIEW (user_data);

        /* Insert the root node */
        value = g_new (NodeValue, 1);
        value->path = g_strdup ("/");
        value->directory = TRUE;
        root = e_tree_model_node_insert (storage_view->priv->model, NULL, -1, value);

	insert_folders_and_files (storage_view, root, "/");

	return (FALSE);
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
//	etable_class->right_click		= right_click;
	etable_class->cursor_change 		= cursor_change;
//	etable_class->table_drag_begin		= table_drag_begin;
//	etable_class->table_drag_data_get	= table_drag_data_get;
//	etable_class->table_drag_motion		= table_drag_motion;
//	etable_class->table_drag_drop		= table_drag_drop;
//	etable_class->table_drag_data_received	= table_drag_data_received;

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
	ECell*			cell;

	new = gtk_type_new (GNOCAM_TYPE_STORAGE_VIEW);
	new->priv->storage = storage;

	/* Create the model */
	new->priv->model = e_tree_simple_new (col_count, NULL, free_value, NULL, NULL, NULL, etree_icon_at, etree_value_at, NULL, etree_is_editable, new);
	gtk_signal_connect (GTK_OBJECT (new->priv->model), "node_expanded", GTK_SIGNAL_FUNC (on_node_expanded), new);

	/* Create extras */
	extras = e_table_extras_new ();
	cell = e_cell_text_new (NULL, GTK_JUSTIFY_LEFT);
	e_table_extras_add_cell (extras, "render_tree", e_cell_tree_new (NULL, NULL, TRUE, cell));

	/* Construct the table */
        e_table_construct (E_TABLE (new), E_TABLE_MODEL (new->priv->model), extras, ETABLE_SPEC, NULL);
        gtk_object_unref (GTK_OBJECT (extras));

	/* D&D */
        e_table_drag_source_set (E_TABLE (new), GDK_BUTTON1_MASK, source_drag_types, num_source_drag_types, GDK_ACTION_MOVE | GDK_ACTION_COPY); 
        e_table_drag_dest_set (E_TABLE (new), GTK_DEST_DEFAULT_ALL, source_drag_types, num_source_drag_types, GDK_ACTION_MOVE | GDK_ACTION_COPY);

	/* Populate the tree */
//	gtk_idle_add (populate_tree, new);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_storage_view, "GnoCamStorageView", GnoCamStorageView, gnocam_storage_view_class_init, gnocam_storage_view_init, PARENT_TYPE)

