#include <gphoto2-camera.h>
#include "gnocam-capplet-table.h"

#include <gal/widgets/e-popup-menu.h>
#include <gal/util/e-util.h>
#include <gal/widgets/e-gui-utils.h>
#include <gphoto2/gphoto2-abilities-list.h>

#include "gnocam-capplet-model.h"
#include "gnocam-configuration.h"

struct _GnoCamCappletTablePrivate
{
	CameraAbilitiesList *al;

	ETableModel *model;
};

#define PARENT_TYPE E_TABLE_TYPE
static ETableClass *parent_class;

#define E_TABLE_SPEC \
"<ETableSpecification click-to-add=\"true\" vertical-draw-grid=\"true\""\
"                     horizontal-draw-grid=\"true\""\
"                  _click-to-add-message=\"* Click here to add a camera *\">"\
"  <ETableColumn model_col=\"0\" _title=\"Name\"  expansion=\"1.0\""\
"                minimum_width=\"20\" resizable=\"true\" cell=\"string\""\
"                compare=\"string\"/>"\
"  <ETableColumn model_col=\"1\" _title=\"Model\" expansion=\"1.0\""\
"                minimum_width=\"20\" resizable=\"true\" cell=\"model\""\
"                compare=\"string\"/>"\
"  <ETableColumn model_col=\"2\" _title=\"Port\"  expansion=\"1.0\""\
"                minimum_width=\"20\" resizable=\"true\" cell=\"port\""\
"                compare=\"string\"/>"\
"  <ETableState>"\
"    <column source=\"0\"/>"\
"    <column source=\"1\"/>"\
"    <column source=\"2\"/>"\
"    <grouping/>"\
"  </ETableState>"\
"</ETableSpecification>"

static void
gnocam_capplet_table_destroy (GtkObject *object)
{
	GnoCamCappletTable *table = GNOCAM_CAPPLET_TABLE (object);

	if (table->priv->al) {
		gp_abilities_list_free (table->priv->al);
		table->priv->al = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_capplet_table_finalize (GtkObject *object)
{
	GnoCamCappletTable *table;

	table = GNOCAM_CAPPLET_TABLE (object);

	g_free (table->priv);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_capplet_table_init (GnoCamCappletTable *table)
{
	table->priv = g_new0 (GnoCamCappletTablePrivate, 1);
}

static void
gnocam_capplet_table_class_init (GnoCamCappletTableClass *klass)
{
	GtkObjectClass *object_class;

	parent_class = gtk_type_class (PARENT_TYPE);

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy  = gnocam_capplet_table_destroy;
	object_class->finalize = gnocam_capplet_table_finalize;
}

static void
configure_camera (GnoCamCappletTable *table, guint number)
{
	Camera *camera;
	CameraAbilities abilities;
	CameraWidget *config;
	const gchar *model, *port;
	GtkWidget *widget;
	gint result;
	int m;

	model = e_table_model_value_at (table->priv->model, 1, number);
	port  = e_table_model_value_at (table->priv->model, 2, number);

	/* Create camera */
	result = gp_camera_new (&camera);
	if (result < 0) {
		g_warning ("Could not create camera: %s",
			   gp_result_as_string (result));
		return;
	}

	/* Set the model */
	m = gp_abilities_list_lookup_model (table->priv->al, model);
	gp_abilities_list_get_abilities (table->priv->al, m, &abilities);
	result = gp_camera_set_abilities (camera, abilities);
	if (result < 0) {
		g_warning ("Could not set model: %s",
			   gp_camera_get_result_as_string (camera, result));
		gp_camera_unref (camera);
		return;
	}

	/* Set the port */
	if (*port) {
		result = gp_camera_set_port_name (camera, port);
		if (result < 0) {
			g_warning ("Could not set port: %s",
				   gp_camera_get_result_as_string (camera,
					   			   result));
			gp_camera_unref (camera);
			return;
		}
	}

	/* Initialize camera */
	result = gp_camera_init (camera);
	if (result < 0) {
		g_warning ("Could not initialize camera: %s",
			   gp_camera_get_result_as_string (camera, result));
		gp_camera_unref (camera);
	}

	/* Get the configuration */
	result = gp_camera_get_config (camera, &config);
	if (result < 0) {
		g_warning ("Could not get configuration: %s",
			   gp_camera_get_result_as_string (camera, result));
		gp_camera_unref (camera);
	}

	widget = gnocam_configuration_new (camera, config);
	gtk_widget_show (widget);

	/* Clean up */
	result = gp_camera_unref (camera);
	if (result < 0)
		g_warning ("Could not unref camera: %s",
			   gp_camera_get_result_as_string (camera, result));
	result = gp_widget_unref (config);
	if (result < 0)
		g_warning ("Could not unref widget: %s",
			   gp_result_as_string (result));
}

static void
get_info (GnoCamCappletTable *table, guint number)
{
	Camera *camera;
	CameraAbilities abilities;
	const gchar *model, *port;
	gint result;
	CameraText text;
	int m;

	model = e_table_model_value_at (table->priv->model, 1, number);
	port  = e_table_model_value_at (table->priv->model, 2, number);

	/* Create camera */
	result = gp_camera_new (&camera);
	if (result < 0) {
		g_warning ("Could not create camera: %s",
			   gp_result_as_string (result));
		return;
	}

	/* Set the model */
	m = gp_abilities_list_lookup_model (table->priv->al, model);
	gp_abilities_list_get_abilities (table->priv->al, m, &abilities);
	result = gp_camera_set_abilities (camera, abilities);
	if (result < 0) {
		g_warning ("Could not set model: %s",
			   gp_camera_get_result_as_string (camera, result));
		gp_camera_unref (camera);
		return;
	}

	/* Set the port */
	if (*port) {
		result = gp_camera_set_port_name (camera, port);
		if (result < 0) {
			g_warning ("Could not set port: %s",
				   gp_camera_get_result_as_string (camera,
					   			   result));
			gp_camera_unref (camera);
			return;
		}
	}

	/* Initialize camera */
	result = gp_camera_init (camera);
	if (result < 0) {
		g_warning ("Could not initialize camera: %s",
			   gp_camera_get_result_as_string (camera, result));
		gp_camera_unref (camera);
	}

	/* Get the summary */
	result = gp_camera_get_summary (camera, &text);
	if (result < 0) {
		g_warning ("Could not get summary: %s",
			   gp_camera_get_result_as_string (camera, result));
	}

	gnome_dialog_run_and_close (GNOME_DIALOG (gnome_ok_dialog (text.text)));

	/* Clean up */
	result = gp_camera_unref (camera);
	if (result < 0)
		g_warning ("Could not unref camera: %s",
			   gp_camera_get_result_as_string (camera, result));
}

static void
delete_camera (GnoCamCappletTable *table, guint number)
{
	GnoCamCappletModel *model;

	model = GNOCAM_CAPPLET_MODEL (table->priv->model);
	gnocam_capplet_model_delete_row (model, number);
}

static void
foreach_delete (gint row, gpointer data)
{
	delete_camera (GNOCAM_CAPPLET_TABLE (data), row);
}

static void
foreach_configure (gint row, gpointer data)
{
	configure_camera (GNOCAM_CAPPLET_TABLE (data), row);
}

static void
foreach_get_info (gint row, gpointer data)
{
	get_info (GNOCAM_CAPPLET_TABLE (data), row);
}

static void
on_delete_clicked (GtkButton *button, gpointer data)
{
	GnoCamCappletTable *table;

	table = GNOCAM_CAPPLET_TABLE (data);

	e_table_selected_row_foreach (E_TABLE (table), foreach_delete, table);
}

static void
on_get_info_clicked (GtkButton *button, gpointer data)
{
	GnoCamCappletTable *table;

	table = GNOCAM_CAPPLET_TABLE (data);

	e_table_selected_row_foreach (E_TABLE (table), foreach_get_info, table);
}

static void
on_configure_clicked (GtkButton *button, gpointer data)
{
	GnoCamCappletTable *table;

	table = GNOCAM_CAPPLET_TABLE (data);

	e_table_selected_row_foreach (E_TABLE (table), foreach_configure,
				      table);
}

static EPopupMenu context_menu [] = {
	{ N_("_Get info"), NULL, GTK_SIGNAL_FUNC (on_get_info_clicked),
	  NULL, 0},
	{ N_("_Configure"), NULL, GTK_SIGNAL_FUNC (on_configure_clicked),
	  NULL, 0},
	{ N_("_Delete"), NULL, GTK_SIGNAL_FUNC (on_delete_clicked), NULL, 0},
	E_POPUP_TERMINATOR
};	

static gint
on_right_click (ETable *et, gint row, gint col, GdkEvent *event, gpointer data)
{
	GnoCamCappletTable *table;
	GtkMenu *menu;

	table = GNOCAM_CAPPLET_TABLE (data);

	menu = e_popup_menu_create (context_menu, 0, 0, table);
	e_auto_kill_popup_menu_on_hide (menu);
	gtk_menu_popup (menu, NULL, NULL, NULL, NULL,
			event->button.button, event->button.time);
	return (TRUE);
}

static gint
on_key_press (ETable *et, gint row, gint col, GdkEvent *event, gpointer data)
{
	GnoCamCappletTable *table;
	GtkMenu *menu;

	table = GNOCAM_CAPPLET_TABLE (data);
	
	switch (event->key.keyval) {
	case GDK_Delete:
	case GDK_KP_Delete:
		delete_camera (table, row);
		return (TRUE);
	case GDK_Menu:
		menu = e_popup_menu_create (context_menu, 0, 0, table);
		e_auto_kill_popup_menu_on_hide (menu);
		gtk_menu_popup (menu, NULL, NULL, NULL, NULL,
				event->button.button, event->button.time);
		return (TRUE);
	default:
		return (FALSE);
	}
}

ETable *
gnocam_capplet_table_new (CappletWidget *capplet)
{
	GnoCamCappletTable *table;
	ETableExtras *extras;
	ECell *cell;
	ECell *popup_cell;
	GList *list;
	gint number, i;
	GPPortInfo info;
	CameraAbilities a;

	table = gtk_type_new (GNOCAM_TYPE_CAPPLET_TABLE);

	gp_abilities_list_new (&(table->priv->al));
	gp_abilities_list_load (table->priv->al);

	/* Create the model */
	table->priv->model = gnocam_capplet_model_new (capplet);

	/* Create the extras */
	extras = e_table_extras_new ();

	/* Create the cell for port */
	cell = e_cell_text_new (NULL, GTK_JUSTIFY_LEFT);
	popup_cell = e_cell_combo_new ();
	list = NULL;
	number = gp_port_core_count ();
	for (i = 0; i < number; i++)
		if (gp_port_core_get_info (i, &info) == GP_OK)
			list = g_list_append (list, g_strdup (info.name));
	e_cell_combo_set_popdown_strings (E_CELL_COMBO (popup_cell), list);
	e_cell_popup_set_child (E_CELL_POPUP (popup_cell), cell);
	gtk_object_unref (GTK_OBJECT (cell));
	e_table_extras_add_cell (extras, "port", popup_cell);

	/* Create the cell for model */
	cell = e_cell_text_new (NULL, GTK_JUSTIFY_LEFT);
	popup_cell = e_cell_combo_new ();
	list = NULL;
	number = gp_abilities_list_count (table->priv->al);
	for (i = 0; i < number; i++) {
		gp_abilities_list_get_abilities (table->priv->al, i, &a);
		list = g_list_append (list, g_strdup (a.model));
	}
	e_cell_combo_set_popdown_strings (E_CELL_COMBO (popup_cell), list);
	e_cell_popup_set_child (E_CELL_POPUP (popup_cell), cell);
	gtk_object_unref (GTK_OBJECT (cell));
	e_table_extras_add_cell (extras, "model", popup_cell); 

	e_table_construct (E_TABLE (table), table->priv->model,
			   extras, E_TABLE_SPEC, NULL);
	gtk_object_unref (GTK_OBJECT (extras));

	gtk_signal_connect (GTK_OBJECT (table), "key_press", 
			    GTK_SIGNAL_FUNC (on_key_press), table);
	gtk_signal_connect (GTK_OBJECT (table), "right_click", 
			    GTK_SIGNAL_FUNC (on_right_click), table);

	return (E_TABLE (table));
}

void
gnocam_capplet_table_ok (GnoCamCappletTable *table)
{
	gnocam_capplet_model_ok (GNOCAM_CAPPLET_MODEL (table->priv->model));
}

void
gnocam_capplet_table_try (GnoCamCappletTable *table)
{
	gnocam_capplet_model_try (GNOCAM_CAPPLET_MODEL (table->priv->model));
}

void
gnocam_capplet_table_cancel (GnoCamCappletTable *table)
{
	gnocam_capplet_model_cancel (GNOCAM_CAPPLET_MODEL (table->priv->model));
}

void
gnocam_capplet_table_revert (GnoCamCappletTable *table)
{
	gnocam_capplet_model_revert (GNOCAM_CAPPLET_MODEL (table->priv->model));
}

E_MAKE_TYPE (gnocam_capplet_table, "GnoCamCappletTable", GnoCamCappletTable, gnocam_capplet_table_class_init, gnocam_capplet_table_init, PARENT_TYPE)


