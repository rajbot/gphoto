#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gphoto2.h>

#include <gnocam-capplet-model.h>

#include <gal/util/e-util.h>
#include <gconf/gconf-client.h>

#define PARENT_TYPE E_TABLE_MODEL_TYPE
static ETableModelClass *parent_class = NULL;

struct _GnoCamCappletModelPrivate 
{
	CappletWidget	*capplet;

	GConfClient	*client;

	GSList		*list;
	GSList		*backup;

	guint		 notify_cameras;
};

static void
notify_cameras (GConfClient *client, guint cnxn_id,
		GConfEntry *entry, gpointer data)
{
	GnoCamCappletModel *model;
	gint		    i;

	model = GNOCAM_CAPPLET_MODEL (data);

	/* Free the current list */ 
	for (i = 0; i < g_slist_length (model->priv->list); i++)
		g_free (g_slist_nth_data (model->priv->list, i)); 
	g_slist_free (model->priv->list); 
	
	/* Get the new list */ 
	model->priv->list = gconf_client_get_list (model->priv->client,
						   "/apps/" PACKAGE "/cameras",
						   GCONF_VALUE_STRING, NULL); 
	
	e_table_model_changed (E_TABLE_MODEL (model)); 

	if (model->priv->capplet)
		capplet_widget_state_changed (model->priv->capplet, TRUE);
}

/*****************/
/* E-Table-Model */
/*****************/

static gint
column_count (ETableModel *m)
{
	return (3);
}

static gint
row_count (ETableModel *m)
{
	GnoCamCappletModel *model;

	model = GNOCAM_CAPPLET_MODEL (m);

	return (g_slist_length (model->priv->list) / 3);
}

static void*
value_at (ETableModel *m, gint col, gint row)
{
	GnoCamCappletModel *model;

	model = GNOCAM_CAPPLET_MODEL (m);

	return (g_strdup (g_slist_nth_data (model->priv->list, 3 * row + col)));
}

static void
set_value_at (ETableModel *m, gint col, gint row, const void* value)
{
	GnoCamCappletModel *model;

	model = GNOCAM_CAPPLET_MODEL (m);

	g_free (g_slist_nth_data (model->priv->list, 3 * row + col));
	(g_slist_nth (model->priv->list, 3 * row + col))->data = g_strdup (value);

	if (model->priv->capplet)
		capplet_widget_state_changed (model->priv->capplet, TRUE);
	else
		gnocam_capplet_model_ok (model);
}

static gboolean
is_cell_editable (ETableModel *m, gint col, gint row)
{
	return (TRUE);
}

static void*
duplicate_value (ETableModel *m, gint col, const void* value)
{
	return (g_strdup (value));
}

static void
free_value (ETableModel *m, gint col, void* value)
{
	g_free (value);
}

static void*
initialize_value (ETableModel *m, gint col)
{
	return (g_strdup (""));
}

static gboolean
value_is_empty (ETableModel *m, gint col, const void* value)
{
	return !(value && *(gchar*)value);
}

static gchar*
value_to_string (ETableModel *m, gint col, const void* value)
{
	return (g_strdup (value));
}

static void
append_row (ETableModel *m, ETableModel *source, gint row)
{
	GnoCamCappletModel *model;
	gint   col;
	gchar *name;

	model = GNOCAM_CAPPLET_MODEL (m);

	for (col = 0; col < 3; col++) {
		name = g_strdup (e_table_model_value_at (source, col, row));
		model->priv->list = g_slist_append (model->priv->list, name);
	}

	e_table_model_changed (m);

	if (model->priv->capplet)
		capplet_widget_state_changed (model->priv->capplet, TRUE);
	else
		gnocam_capplet_model_ok (model);
}

/*******************/
/* GtkObject stuff */
/*******************/

void
gnocam_capplet_model_ok (GnoCamCappletModel *model)
{
	gconf_client_set_list (model->priv->client,
			       "/apps/" PACKAGE "/cameras",
			       GCONF_VALUE_STRING, model->priv->list, NULL);
}

void
gnocam_capplet_model_cancel (GnoCamCappletModel *model)
{
	gconf_client_set_list (model->priv->client,
			       "/apps/" PACKAGE "/cameras",
			       GCONF_VALUE_STRING, model->priv->backup, NULL);
}

void
gnocam_capplet_model_try (GnoCamCappletModel *model)
{
	gconf_client_set_list (model->priv->client,
		               "/apps/" PACKAGE "/cameras",
			       GCONF_VALUE_STRING, model->priv->list, NULL);
}

void
gnocam_capplet_model_revert (GnoCamCappletModel* model)
{
	gint i;

	/* Free the current list */ 
	for (i = 0; i < g_slist_length (model->priv->list); i++)
		g_free (g_slist_nth_data (model->priv->list, i)); 
	g_slist_free (model->priv->list); 
	
	gconf_client_set_list (model->priv->client,
			       "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING,
			       model->priv->backup, NULL); 
	model->priv->list = gconf_client_get_list (model->priv->client,
						   "/apps/" PACKAGE "/cameras",
						   GCONF_VALUE_STRING, NULL); 
	
	e_table_model_changed (E_TABLE_MODEL (model));
}
							
void
gnocam_capplet_model_delete_row (GnoCamCappletModel *model, gint row)
{
	GSList *link; 
	gint    i;

	if (3 * (row + 1) > g_slist_length (model->priv->list)) {
		g_warning ("Deletion of multiple rows at once not yet supported.");
		return;
	}

	for (i = 0; i < 3; i++) { 
		link = g_slist_nth (model->priv->list, 3 * row); 
		model->priv->list = g_slist_remove_link (model->priv->list, link); 
		g_free (link->data); 
		g_slist_free_1 (link); 
	} 
	
	e_table_model_changed (E_TABLE_MODEL (model)); 

	if (model->priv->capplet)
		capplet_widget_state_changed (model->priv->capplet, TRUE);
	else
		gnocam_capplet_model_ok (model);
}

static void
gnocam_capplet_model_destroy (GtkObject *object)
{
	GnoCamCappletModel *model;
	gint		    i;

	model = GNOCAM_CAPPLET_MODEL (object);

	if (model->priv->list) {
		for (i = 0; i < g_slist_length (model->priv->list); i++)
			g_free (g_slist_nth_data (model->priv->list, i)); 
		g_slist_free (model->priv->list); 
		model->priv->list = NULL;
	}
	
	if (model->priv->backup) {
		for (i = 0; i < g_slist_length (model->priv->backup); i++)
			g_free (g_slist_nth_data (model->priv->backup, i)); 
		g_slist_free (model->priv->backup);
		model->priv->backup = NULL;
	}
				
	if (model->priv->notify_cameras) {
		gconf_client_notify_remove (model->priv->client,
					    model->priv->notify_cameras);
		model->priv->notify_cameras = 0;
	}

	if (model->priv->client) {
		gtk_object_unref (GTK_OBJECT (model->priv->client));
		model->priv->client = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_capplet_model_finalize (GtkObject *object)
{
	GnoCamCappletModel *model;

	model = GNOCAM_CAPPLET_MODEL (object);

	g_free (model->priv);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_capplet_model_init (GnoCamCappletModel *model)
{
	model->priv = g_new0 (GnoCamCappletModelPrivate, 1);
}

static void
gnocam_capplet_model_class_init (GnoCamCappletModelClass *klass)
{
	GtkObjectClass 		*object_class;
	ETableModelClass	*model_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy  = gnocam_capplet_model_destroy;
	object_class->finalize = gnocam_capplet_model_finalize;

	model_class = E_TABLE_MODEL_CLASS (klass);
	model_class->column_count = column_count;
	model_class->row_count = row_count;
	model_class->value_at = value_at;
	model_class->set_value_at = set_value_at;
	model_class->is_cell_editable = is_cell_editable;
	model_class->append_row = append_row;
	model_class->duplicate_value = duplicate_value;
	model_class->free_value = free_value;
	model_class->initialize_value = initialize_value;
	model_class->value_is_empty = value_is_empty;
	model_class->value_to_string = value_to_string;

	parent_class = gtk_type_class (PARENT_TYPE);
}

ETableModel*
gnocam_capplet_model_new (CappletWidget *capplet)
{
	GnoCamCappletModel *model;

	model = gtk_type_new (GNOCAM_TYPE_CAPPLET_MODEL);
	model->priv->capplet = capplet;

	model->priv->client = gconf_client_get_default ();
	gconf_client_add_dir (model->priv->client, "/apps/" PACKAGE,
			      GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);
	model->priv->list = gconf_client_get_list (model->priv->client,
						   "/apps/" PACKAGE "/cameras",
						   GCONF_VALUE_STRING, NULL);
	model->priv->backup = gconf_client_get_list (model->priv->client,
			"/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, NULL);
	gconf_client_notify_add (model->priv->client,
				 "/apps/" PACKAGE "/cameras", notify_cameras,
				 model, NULL, NULL);

	return (E_TABLE_MODEL (model));
}

E_MAKE_TYPE (gnocam_capplet_model, "GnoCamCappletModel", GnoCamCappletModel, gnocam_capplet_model_class_init, gnocam_capplet_model_init, PARENT_TYPE)

