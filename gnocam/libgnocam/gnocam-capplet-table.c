#include <gphoto2.h>
#include "gnocam-capplet-table.h"

#include <gal/widgets/e-popup-menu.h>
#include <gal/util/e-util.h>
#include <gal/widgets/e-gui-utils.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-object.h>
#include <liboaf/oaf-activate.h>

#include "gnocam-capplet-model.h"
#include "GnoCam.h"

struct _GnoCamCappletTablePrivate
{
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
	GnoCamCappletTable *table;

	table = GNOCAM_CAPPLET_TABLE (object);

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

static GNOME_Camera
get_camera (GnoCamCappletTable *table, guint number)
{
	const gchar *name;
	CORBA_Environment ev;
	GNOME_Camera camera;
	CORBA_Object gnocam;

	name = name = e_table_model_value_at (table->priv->model, 0, number);
	
	CORBA_exception_init (&ev);
	gnocam = oaf_activate_from_id ("OAFIID:GNOME_GnoCam", 0, NULL, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not start GnoCam: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return (CORBA_OBJECT_NIL);
	}

	camera = GNOME_GnoCam_getCameraByName (gnocam, name, &ev);
	bonobo_object_release_unref (gnocam, NULL);
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not get camera: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return (CORBA_OBJECT_NIL);
	}

	return (camera);
}

static void
configure_camera (GnoCamCappletTable *table, guint number)
{	
	GNOME_Camera camera;
	CORBA_Environment ev;

	camera = get_camera (table, number);
	if (camera == CORBA_OBJECT_NIL)
		return;

	CORBA_exception_init (&ev);
	GNOME_Camera_showConfiguration (camera, &ev);
	bonobo_object_release_unref (camera, NULL);
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not show configuration: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return;
	}
	CORBA_exception_free (&ev);
}

static void
get_info (GnoCamCappletTable *table, guint number)
{
	GNOME_Camera camera;
	CORBA_Environment ev;
	CORBA_char *info;

	camera = get_camera (table, number);
	if (camera == CORBA_OBJECT_NIL)
		return;

	CORBA_exception_init (&ev);
	info = GNOME_Camera_getInfo (camera, &ev);
	bonobo_object_release_unref (camera, NULL);
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not get info: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return;
		        }
	CORBA_exception_free (&ev);

	gnome_ok_dialog (info);

	CORBA_free (info);
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
	gtk_menu_popup (menu, NULL, NULL, NULL, NULL, 0, event->button.time);
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
	gchar buffer [1024];
	CameraPortInfo info;

	table = gtk_type_new (GNOCAM_TYPE_CAPPLET_TABLE);

	/* Create the model */
	table->priv->model = gnocam_capplet_model_new (capplet);

	/* Create the extras */
	extras = e_table_extras_new ();

	/* Create the cell for port */
	cell = e_cell_text_new (NULL, GTK_JUSTIFY_LEFT);
	popup_cell = e_cell_combo_new ();
	list = NULL;
	number = gp_port_count_get ();
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
	number = gp_camera_count ();
	for (i = 0; i < number; i++)
		if (gp_camera_name (i, buffer) == GP_OK)
			list = g_list_append (list, g_strdup (buffer));
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


