#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gphoto2.h>

#include "gnocam-capplet-content.h"

#include <gal/util/e-util.h>
#include <gal/e-table/e-table.h>
#include <gal/e-table/e-table-simple.h>
#include <gal/e-table/e-table-scrolled.h>
#include <gal/e-table/e-cell-text.h>
#include <gal/e-table/e-cell-popup.h>
#include <gal/e-table/e-cell-combo.h>
#include <gconf/gconf-client.h>

#define PARENT_TYPE GTK_TYPE_VBOX
static GtkVBoxClass* parent_class = NULL;

struct _GnoCamCappletContentPrivate {

	GConfClient*	client;
	guint		cnxn;

	CappletWidget*	capplet;

	GSList*		list;
	GSList*		backup;

	ETableModel*	model;
	GtkWidget*	table;
};

#define E_TABLE_SPEC                                                                                                                                            \
"<ETableSpecification click-to-add=\"true\" draw-grid=\"true\" _click-to-add-message=\"* Click here to add a camera *\">"					\
"  <ETableColumn model_col=\"0\" _title=\"Name\"  expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\" compare=\"string\"/>"		\
"  <ETableColumn model_col=\"1\" _title=\"Model\" expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"model\" compare=\"string\"/>"		\
"  <ETableColumn model_col=\"2\" _title=\"Port\"  expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"port\" compare=\"string\"/>"			\
"  <ETableState>"                                                                                                                                               \
"    <column source=\"0\"/>"                                                                                                                                    \
"    <column source=\"1\"/>"                                                                                                                                    \
"    <column source=\"2\"/>"                                                                                                                                    \
"    <grouping/>"                                                                                                                                               \
"  </ETableState>"                                                                                                                                              \
"</ETableSpecification>"

/******************/
/* Internal stuff */
/******************/

static void
notify_cameras (GConfClient* client, guint cnxn_id, GConfEntry* entry, gpointer user_data)
{
	GnoCamCappletContent*	content;
	gint			i;

	content = GNOCAM_CAPPLET_CONTENT (user_data);

	/* Free the current list */
	for (i = 0; i < g_slist_length (content->priv->list); i++) g_free (g_slist_nth_data (content->priv->list, i));
	g_slist_free (content->priv->list);

	/* Get the new list */
	content->priv->list = gconf_client_get_list (content->priv->client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, NULL);

	e_table_model_changed (content->priv->model);
	capplet_widget_state_changed (content->priv->capplet, TRUE);
}

static void
table_selected_row_foreach_delete (int row, gpointer user_data)
{
	GnoCamCappletContent*	content;
	GSList*			link;
	gint			i;

	content = GNOCAM_CAPPLET_CONTENT (user_data);

	for (i = 0; i < 3; i++) {
		link = g_slist_nth (content->priv->list, 3 * row);
		content->priv->list = g_slist_remove_link (content->priv->list, link);
		g_free (link->data);
		g_slist_free_1 (link);
	}

	e_table_model_changed (content->priv->model);
	capplet_widget_state_changed (content->priv->capplet, TRUE);
}

/*************/
/* Callbacks */
/*************/

static void
on_delete_clicked (GtkButton* button, gpointer user_data)
{
	GnoCamCappletContent*	content;

	content = GNOCAM_CAPPLET_CONTENT (user_data);

	e_table_selected_row_foreach (e_table_scrolled_get_table (E_TABLE_SCROLLED (content->priv->table)), table_selected_row_foreach_delete, content);
}

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
	GnoCamCappletContent*	content;

	content = GNOCAM_CAPPLET_CONTENT (user_data);

	return (g_slist_length (content->priv->list) / 3);
}

static void*
value_at (ETableModel* model, gint col, gint row, gpointer user_data)
{
	GnoCamCappletContent*	content;

	content = GNOCAM_CAPPLET_CONTENT (user_data);

	return (g_strdup (g_slist_nth_data (content->priv->list, 3 * row + col)));
}

static void
set_value_at (ETableModel* model, gint col, gint row, const void* value, gpointer user_data)
{
	GnoCamCappletContent*	content;

	content = GNOCAM_CAPPLET_CONTENT (user_data);

	g_free (g_slist_nth_data (content->priv->list, 3 * row + col));
	(g_slist_nth (content->priv->list, 3 * row + col))->data = g_strdup (value);

	capplet_widget_state_changed (content->priv->capplet, TRUE);
}

static gboolean
is_cell_editable (ETableModel* model, gint col, gint row, gpointer user_data)
{
        return (TRUE);
}

static void*
duplicate_value (ETableModel* model, gint col, const void* value, gpointer user_data)
{
        return (g_strdup (value));
}

static void
free_value (ETableModel* model, gint col, void* value, gpointer user_data)
{
	g_free (value);
}

static void*
initialize_value (ETableModel* model, gint col, gpointer user_data)
{
        return (g_strdup (""));
}

static gboolean
value_is_empty (ETableModel* model, gint col, const void* value, gpointer user_data)
{
	return !(value && *(gchar*)value);
}

static gchar*
value_to_string (ETableModel* model, gint col, const void* value, gpointer user_data)
{
        return (g_strdup (value));
}

static void
append_row (ETableModel* model, ETableModel* source, int row, gpointer user_data)
{
	GnoCamCappletContent*	content;
	gint			col;

	g_return_if_fail (user_data);
	content = GNOCAM_CAPPLET_CONTENT (user_data);

	for (col = 0; col < 3; col++) g_slist_append (content->priv->list, g_strdup (e_table_model_value_at (source, col, row)));
	
	e_table_model_changed (model);
	capplet_widget_state_changed (content->priv->capplet, TRUE);
}

/*****************/
/* Our functions */
/*****************/

void
gnocam_capplet_content_ok (GnoCamCappletContent* content)
{
	gconf_client_set_list (content->priv->client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, content->priv->list, NULL);
}

void
gnocam_capplet_content_revert (GnoCamCappletContent* content)
{
	gint	i;

	/* Free the current list */
	for (i = 0; i < g_slist_length (content->priv->list); i++) g_free (g_slist_nth_data (content->priv->list, i));
	g_slist_free (content->priv->list);

	gconf_client_set_list (content->priv->client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, content->priv->backup, NULL);
	content->priv->list = gconf_client_get_list (content->priv->client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, NULL);

	e_table_model_changed (content->priv->model);
}

void
gnocam_capplet_content_try (GnoCamCappletContent* content)
{
	gconf_client_set_list (content->priv->client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, content->priv->list, NULL);
}

void
gnocam_capplet_content_cancel (GnoCamCappletContent* content)
{
	gconf_client_set_list (content->priv->client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, content->priv->backup, NULL);
}

/*************/
/* Gtk stuff */
/*************/

static void
gnocam_capplet_content_destroy (GtkObject* object)
{
	GnoCamCappletContent*	content;
	gint			i;

	content = GNOCAM_CAPPLET_CONTENT (object);

	gtk_object_unref (GTK_OBJECT (content->priv->client));

	for (i = 0; i < g_slist_length (content->priv->list); i++) g_free (g_slist_nth_data (content->priv->list, i));
	g_slist_free (content->priv->list);

	for (i = 0; i < g_slist_length (content->priv->backup); i++) g_free (g_slist_nth_data (content->priv->backup, i));
	g_slist_free (content->priv->backup);
	
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
gnocam_capplet_content_new (CappletWidget* capplet)
{
	GdkPixbuf*		pixbuf;
	GdkPixmap*		pixmap = NULL;
	GdkBitmap*		bitmap = NULL;
	GnoCamCappletContent*	new;
	GtkWidget*		hbox;
	GtkWidget*		vbuttonbox;
	GtkWidget*		button;
	GtkWidget*		widget;
	ETableExtras*		extras;
	ECell*			cell;
	ECell*			popup_cell;
	gint			number;
	gint			i;
	gchar			buffer [1024];
	GList*			list;
	CameraPortInfo		info;

	new = gtk_type_new (GNOCAM_TYPE_CAPPLET_CONTENT);
	
	new->priv->capplet = capplet;
	new->priv->client = gconf_client_get_default ();
	gconf_client_notify_add (new->priv->client, "/apps/" PACKAGE "/cameras", notify_cameras, new, NULL, NULL);
	gconf_client_add_dir (new->priv->client, "/apps/" PACKAGE, GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);
	new->priv->list = gconf_client_get_list (new->priv->client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, NULL);
	new->priv->backup = gconf_client_get_list (new->priv->client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, NULL);

	/* Create a hbox */
	hbox = gtk_hbox_new (FALSE, 10);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (new), hbox, FALSE, FALSE, 10);

	/* Create the logo */
	pixbuf = gdk_pixbuf_new_from_file (IMAGEDIR "/gnocam.png");
	gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap, 4);
	widget = gtk_pixmap_new (pixmap, bitmap);
	gtk_widget_show (widget);
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 10);

	/* Create the hbox */
	hbox = gtk_hbox_new (FALSE, 10);
	gtk_widget_show (hbox);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
	gtk_box_pack_start (GTK_BOX (new), hbox, TRUE, TRUE, 10);

	/* Create the model */
	new->priv->model = e_table_simple_new (col_count, row_count, value_at, set_value_at, is_cell_editable, 
		duplicate_value, free_value, initialize_value, value_is_empty, value_to_string, new);
	E_TABLE_SIMPLE (new->priv->model)->append_row = append_row;

	/* Create the extras */
        extras = e_table_extras_new ();

	/* Create the cell for port */
	cell = e_cell_text_new (NULL, GTK_JUSTIFY_LEFT);
	popup_cell = e_cell_combo_new ();
	list = NULL;
	if ((number = gp_port_count_get ()) >= 0) 
		for (i = 0; i < number; i++) 
			if (gp_port_info_get (i, &info) == GP_OK) 
				list = g_list_append (list, g_strdup (info.name));
	e_cell_combo_set_popdown_strings (E_CELL_COMBO (popup_cell), list);
	e_cell_popup_set_child (E_CELL_POPUP (popup_cell), cell);
	gtk_object_unref (GTK_OBJECT (cell));
	e_table_extras_add_cell (extras, "port", popup_cell);

	/* Create the cell for model */
	cell = e_cell_text_new (NULL, GTK_JUSTIFY_LEFT);
	popup_cell = e_cell_combo_new ();
	list = NULL;
        if ((number = gp_camera_count ()) >= 0) 
		for (i = 0; i < number; i++) 
			if (gp_camera_name (i, buffer) == GP_OK) 
				list = g_list_append (list, g_strdup (buffer));
	e_cell_combo_set_popdown_strings (E_CELL_COMBO (popup_cell), list);
	e_cell_popup_set_child (E_CELL_POPUP (popup_cell), cell);
	gtk_object_unref (GTK_OBJECT (cell));
	e_table_extras_add_cell (extras, "model", popup_cell);

	/* Create the table */
	new->priv->table = e_table_scrolled_new (new->priv->model, extras, E_TABLE_SPEC, NULL);
	gtk_object_unref (GTK_OBJECT (extras));
	gtk_widget_show (new->priv->table);
	gtk_box_pack_start (GTK_BOX (hbox), new->priv->table, TRUE, TRUE, 10);

	/* Create the buttons */
	vbuttonbox = gtk_vbutton_box_new ();
	gtk_button_box_set_layout (GTK_BUTTON_BOX (vbuttonbox), GTK_BUTTONBOX_START);
	gtk_widget_show (vbuttonbox);
	gtk_box_pack_end (GTK_BOX (hbox), vbuttonbox, FALSE, FALSE, 10);
	button = gtk_button_new_with_label (_("Delete"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (on_delete_clicked), new);
	gtk_widget_show (button);
	gtk_box_pack_end (GTK_BOX (vbuttonbox), button, FALSE, FALSE, 10);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_capplet_content, "GnoCamCappletContent", GnoCamCappletContent, gnocam_capplet_content_class_init, gnocam_capplet_content_init, PARENT_TYPE)

