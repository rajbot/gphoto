#include "config.h"
#include "GnoCam.h"
#include "gnocam-applet-marshal.h"
#include "gnocam-prefs.h"
#include "i18n.h"

#include <stdlib.h>
#include <string.h>

#include <gtk/gtkentry.h>
#include <gtk/gtktable.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkeditable.h>

#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-object.h>

#define PARENT_TYPE GTK_TYPE_DIALOG
static GtkDialogClass *parent_class;

enum {
	CAMERA_CHANGED,
	CONNECT_CHANGED,
	MODEL_CHANGED,
	PORT_CHANGED,
	LAST_SIGNAL
};

static gint signals[LAST_SIGNAL] = {0};

struct _GnoCamPrefsPrivate {
	GtkWidget *combo_model;
	GtkWidget *combo_port;
	GtkWidget *check_camera;
	GtkWidget *check_connect;
};

static void
gnocam_prefs_finalize (GObject *object)
{
	GnoCamPrefs *p = GNOCAM_PREFS (object);

	g_free (p->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_prefs_class_init (gpointer g_class, gpointer class_data)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gnocam_prefs_finalize;

	signals[CAMERA_CHANGED] = g_signal_new ("camera_changed",
		G_TYPE_FROM_CLASS (g_class), G_SIGNAL_RUN_FIRST, 
		G_STRUCT_OFFSET (GnoCamPrefsClass, camera_changed),
		NULL, NULL, gnocam_applet_marshal_VOID__BOOLEAN,
		G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
	signals[CONNECT_CHANGED] = g_signal_new ("connect_changed", 
		G_TYPE_FROM_CLASS (g_class), G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET (GnoCamPrefsClass, connect_changed),
		NULL, NULL, gnocam_applet_marshal_VOID__BOOLEAN,
		G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
	signals[MODEL_CHANGED] = g_signal_new ("model_changed",
		G_TYPE_FROM_CLASS (g_class), G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GnoCamPrefsClass, model_changed),
		NULL, NULL, gnocam_applet_marshal_VOID__STRING,
		G_TYPE_NONE, 1, G_TYPE_STRING);
	signals[PORT_CHANGED] = g_signal_new ("port_changed",
		G_TYPE_FROM_CLASS (g_class), G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GnoCamPrefsClass, port_changed),
		NULL, NULL, gnocam_applet_marshal_VOID__STRING,
		G_TYPE_NONE, 1, G_TYPE_STRING);

	parent_class = g_type_class_peek_parent (g_class);
}

static void
on_close_clicked (GtkButton *button, GnoCamPrefs *p)
{
	gtk_widget_destroy (GTK_WIDGET (p));
}

static void
on_camera_toggled (GtkToggleButton *button, GnoCamPrefs *p)
{
	g_return_if_fail (GNOCAM_IS_PREFS (p));

	gtk_widget_set_sensitive (p->priv->combo_model, !button->active);
	gtk_widget_set_sensitive (p->priv->combo_port, !button->active);

	g_signal_emit (p, signals[CAMERA_CHANGED], 0, button->active);
}

static void
on_connect_toggled (GtkToggleButton *button, GnoCamPrefs *p)
{
	g_return_if_fail (GNOCAM_IS_PREFS (p));

	g_signal_emit (p, signals[CONNECT_CHANGED], 0, button->active);
}

static void
on_model_changed (GtkEntry *e, GnoCamPrefs *p)
{
	g_return_if_fail (GNOCAM_IS_PREFS (p));
	g_return_if_fail (GTK_IS_ENTRY (e));

	g_signal_emit (p, signals[MODEL_CHANGED], 0, gtk_entry_get_text (e));
}

static void
on_port_changed (GtkEntry *e, GnoCamPrefs *p)
{
	g_return_if_fail (GNOCAM_IS_PREFS (p));
	g_return_if_fail (GTK_IS_ENTRY (e));

	g_signal_emit (p, signals[PORT_CHANGED], 0, gtk_entry_get_text (e));
}

static void
gnocam_prefs_init (GTypeInstance *instance, gpointer g_class)
{
	GnoCamPrefs *p = GNOCAM_PREFS (instance);
	GtkWidget *button, *label, *t;

	p->priv = g_new0 (GnoCamPrefsPrivate, 1);

	/* Set up the dialog */
	gtk_container_set_border_width (
			GTK_CONTAINER (GTK_DIALOG (p)->vbox), 5);

	p->priv->check_connect = gtk_check_button_new_with_label (
					_("Connect automatically"));
	gtk_widget_show (p->priv->check_connect);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (p)->vbox), 
				p->priv->check_connect, FALSE, FALSE, 0);
	p->priv->check_camera = gtk_check_button_new_with_label (
					 _("Detect camera automatically"));
	gtk_widget_show (p->priv->check_camera);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (p)->vbox),
				p->priv->check_camera, FALSE, FALSE, 0);

	t = gtk_table_new (2, 2, FALSE);
	gtk_widget_show (t);
	gtk_table_set_col_spacings (GTK_TABLE (t), 5);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (p)->vbox), t, FALSE, FALSE, 0);

	/* Model */
	p->priv->combo_model = gtk_combo_new ();
	gtk_widget_show (p->priv->combo_model);
	gtk_table_attach_defaults (GTK_TABLE (t), p->priv->combo_model,
				   1, 2, 0, 1);
	label = gtk_label_new (_("Model:"));
	gtk_widget_show (label);
	gtk_table_attach_defaults (GTK_TABLE (t), label, 0, 1, 0, 1);

	/* Port */
	p->priv->combo_port = gtk_combo_new ();
	gtk_widget_show (p->priv->combo_port);
	gtk_table_attach_defaults (GTK_TABLE (t), p->priv->combo_port,
				   1, 2, 1, 2);
	label = gtk_label_new (_("Port:"));
	gtk_widget_show (label);
	gtk_table_attach_defaults (GTK_TABLE (t), label, 0, 1, 1, 2);

	/* Close button. */
	button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_widget_show (button);
	gtk_box_pack_end (GTK_BOX (GTK_DIALOG (p)->action_area), button,
			  FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (on_close_clicked), p);
}

GType
gnocam_prefs_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo ti;

		memset (&ti, 0, sizeof (GTypeInfo));
		ti.class_size    = sizeof (GnoCamPrefsClass);
		ti.class_init    = gnocam_prefs_class_init;
		ti.instance_size = sizeof (GnoCamPrefs);
		ti.instance_init = gnocam_prefs_init;

		type = g_type_register_static (PARENT_TYPE, "GnoCamPrefs",
					       &ti, 0);
	}

	return (type);
}

GnoCamPrefs *
gnocam_prefs_new (gboolean camera_automatic, gboolean connect_automatic,
		  const gchar *model, const gchar *port, CORBA_Environment *ev)
{
	GnoCamPrefs *p;
	Bonobo_Unknown o;
	GNOME_GnoCam_PortList *port_list;
	GNOME_GnoCam_ModelList *model_list;
	GList *l;
	guint i;

	g_return_val_if_fail (ev != NULL, NULL);

	o = bonobo_get_object ("OAFIID:GNOME_GnoCam",
			       "Bonobo/Unknown", ev);
	if (BONOBO_EX (ev) || (o == CORBA_OBJECT_NIL))
		return NULL;

	port_list = GNOME_GnoCam_getPortList (o, ev);
	if (BONOBO_EX (ev)) {
		bonobo_object_release_unref (o, NULL);
		return NULL;
	}

	model_list = GNOME_GnoCam_getModelList (o, ev);
	if (BONOBO_EX (ev)) {
		CORBA_free (port_list);
		bonobo_object_release_unref (o, NULL);
		return NULL;
	}

	p = g_object_new (GNOCAM_TYPE_PREFS, NULL);

	/* Set up the model combo. */
	for (l = NULL, i = 0; i < model_list->_length; i++)
		l = g_list_append (l, g_strdup (model_list->_buffer[i]));
	if (l)
		gtk_combo_set_popdown_strings (
				GTK_COMBO (p->priv->combo_model), l);
	CORBA_free (model_list);
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (p->priv->combo_model)->entry),
		model ? model : "");

	/* Set up the port combo. */
	for (l = NULL, i = 0; i < port_list->_length; i++)
		l = g_list_append (l, g_strdup (port_list->_buffer[i]));
	if (l)
		gtk_combo_set_popdown_strings (
				GTK_COMBO (p->priv->combo_port), l);
	CORBA_free (port_list);
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (p->priv->combo_port)->entry),
		port ? port : "");

	bonobo_object_release_unref (o, NULL);

	gtk_toggle_button_set_active (
		GTK_TOGGLE_BUTTON (p->priv->check_camera), camera_automatic);
	if (camera_automatic) {
		gtk_widget_set_sensitive (p->priv->combo_model, FALSE);
		gtk_widget_set_sensitive (p->priv->combo_port, FALSE);
	}
	gtk_toggle_button_set_active (
		GTK_TOGGLE_BUTTON (p->priv->check_connect), connect_automatic);

	/* Connect the signals. */
	g_signal_connect (G_OBJECT (p->priv->check_camera),
		"toggled", G_CALLBACK (on_camera_toggled), p);
	g_signal_connect (p->priv->check_connect,
		"toggled", G_CALLBACK (on_connect_toggled), p);
	g_signal_connect (GTK_COMBO (p->priv->combo_model)->entry,
		"changed", G_CALLBACK (on_model_changed), p);
	g_signal_connect (GTK_COMBO (p->priv->combo_port)->entry,
		"changed", G_CALLBACK (on_port_changed), p);

	return p;
}
