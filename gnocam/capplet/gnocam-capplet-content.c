#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-capplet-content.h"

#include <gal/util/e-util.h>
#include <gal/e-table/e-table.h>
#include <gal/e-table/e-table-simple.h>
#include <gal/e-table/e-cell-combo.h>
#include <gconf/gconf-client.h>

#define PARENT_TYPE GTK_TYPE_VBOX
static GtkVBoxClass* parent_class = NULL;

struct _GnoCamCappletContentPrivate {
	GConfClient*	client;

	GtkWidget*	table;
};

#define E_TABLE_SPEC                                                                                                                                            \
"<ETableSpecification>"                                                                                                                                         \
"  <ETableColumn model_col=\"0\" _title=\"Name\"  expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\" compare=\"string\"/>"		\
"  <ETableColumn model_col=\"1\" _title=\"Model\" expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"combo_model\" compare=\"string\"/>"		\
"  <ETableColumn model_col=\"2\" _title=\"Port\"  expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"combo_port\" compare=\"string\"/>"		\
"  <ETableState>"                                                                                                                                               \
"    <column source=\"0\"/>"                                                                                                                                    \
"    <column source=\"1\"/>"                                                                                                                                    \
"    <column source=\"2\"/>"                                                                                                                                    \
"    <grouping/>"                                                                                                                                               \
"  </ETableState>"                                                                                                                                              \
"</ETableSpecification>"

/***********/
/* E-Table */
/***********/

static gint
col_count (ETableModel* model, gpointer user_data)
{
        return (3);
}

static gint
row_count (ETableModel* model, gpointer user_data)
{
	return (2);
}

static void*
value_at (ETableModel* model, gint col, gint row, gpointer user_data)
{
	switch (col) {
	case 0:
		return (g_strdup ("Insert name here..."));
	case 1:
		return (g_strdup ("Insert model here..."));
	case 2:
		return (g_strdup ("Insert port here..."));
	default:
		return NULL;
	}
}

static void
set_value_at (ETableModel* model, gint col, gint row, const void* value, gpointer user_data)
{
}

static gboolean
is_cell_editable (ETableModel* model, gint col, gint row, gpointer user_data)
{
        return (TRUE);
}

static void*
duplicate_value (ETableModel* model, gint col, const void* value, gpointer user_data)
{
        return (NULL);
}

static void
free_value (ETableModel* model, gint col, void* value, gpointer user_data)
{
}

static void*
initialize_value (ETableModel* model, gint col, gpointer user_data)
{
        return (NULL);
}

static gboolean
value_is_empty (ETableModel* model, gint col, const void* value, gpointer user_data)
{
        return (FALSE);
}

static gchar*
value_to_string (ETableModel* model, gint col, const void* value, gpointer user_data)
{
        return (NULL);
}

/*************/
/* Gtk stuff */
/*************/

static void
gnocam_capplet_content_destroy (GtkObject* object)
{
	GnoCamCappletContent*	content;

	content = GNOCAM_CAPPLET_CONTENT (object);

	gtk_object_unref (GTK_OBJECT (content->priv->client));
	
	g_free (content->priv);
	content->priv = NULL;

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_capplet_content_class_init (GnoCamCappletContentClass* klass)
{
	GtkObjectClass*         object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = gnocam_capplet_content_destroy;

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_capplet_content_init (GnoCamCappletContent* content)
{
	content->priv = g_new0 (GnoCamCappletContentPrivate, 1);
}

GtkWidget*
gnocam_capplet_content_new (void)
{
	GnoCamCappletContent*	new;
	ETableModel*		model;
	ETableExtras*		extras;
	ECell*			cell;
	GList*			list;

	new = gtk_type_new (GNOCAM_TYPE_CAPPLET_CONTENT);
	gtk_box_set_homogeneous (GTK_BOX (new), FALSE);
	gtk_box_set_spacing (GTK_BOX (new), 10);
	gtk_container_set_border_width (GTK_CONTAINER (new), 10);
	
	new->priv->client = gconf_client_get_default ();

	/* Create the table */
	model = e_table_simple_new (col_count, row_count, value_at, set_value_at, is_cell_editable, 
		duplicate_value, free_value, initialize_value, value_is_empty, value_to_string, new);

	/* Create the extras */
        extras = e_table_extras_new ();

	/* Create the cell for port */
	list = g_list_append (NULL, g_strdup ("Insert port 1 here..."));
	list = g_list_append (list, g_strdup ("Insert port 2 here..."));
	cell = e_cell_combo_new ();
	e_cell_combo_set_popdown_strings (E_CELL_COMBO (cell), list);
	e_table_extras_add_cell (extras, "combo_port", cell);

	/* Create the cell for model */
	list = g_list_append (NULL, g_strdup ("Insert model 1 here..."));
	list = g_list_append (list, g_strdup ("Insert model 2 here..."));
	cell = e_cell_combo_new ();
	e_cell_combo_set_popdown_strings (E_CELL_COMBO (cell), list);
	e_table_extras_add_cell (extras, "combo_model", cell);

	new->priv->table = e_table_new (model, extras, E_TABLE_SPEC, NULL);
	gtk_object_unref (GTK_OBJECT (extras));
	gtk_widget_show (new->priv->table);
	gtk_container_add (GTK_CONTAINER (new), new->priv->table);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_capplet_content, "GnoCamCappletContent", GnoCamCappletContent, gnocam_capplet_content_class_init, gnocam_capplet_content_init, PARENT_TYPE)

