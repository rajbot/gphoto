#include <config.h>
#include "gnocam-chooser.h"

#include "GNOME_C.h"

#include <gtk/gtkbox.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkcontainer.h>
#include <gtk/gtkstock.h>

#include <glade/glade.h>

#include <bonobo-activation/bonobo-activation-activate.h>

#include <bonobo/bonobo-exception.h>

struct _GnocamChooserPriv
{
};

#define PARENT_TYPE GTK_TYPE_DIALOG
static GtkDialogClass *parent_class;

static void
gnocam_chooser_finalize (GObject *object)
{
	GnocamChooser *c = GNOCAM_CHOOSER (object);

	g_free (c->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_chooser_class_init (gpointer g_class, gpointer class_data)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gnocam_chooser_finalize;

	parent_class = g_type_class_peek_parent (g_class);
}

static void
gnocam_chooser_init (GTypeInstance *instance, gpointer g_class)
{
	GnocamChooser *c = GNOCAM_CHOOSER (instance);

	c->priv = g_new0 (GnocamChooserPriv, 1);
}

GType
gnocam_chooser_get_type (void)
{
	static GType t = 0;

	if (!t) {
		GTypeInfo ti;

		memset (&ti, 0, sizeof (GTypeInfo));
		ti.class_size    = sizeof (GnocamChooserClass);
		ti.class_init    = gnocam_chooser_class_init;
		ti.instance_size = sizeof (GnocamChooser);
		ti.instance_init = gnocam_chooser_init;

		t = g_type_register_static (PARENT_TYPE, "GnocamChooser", &ti,
					    0);
	}

	return t;
}

static void
gnocam_chooser_setup (GnocamChooser *c)
{
	Bonobo_ServerInfoList *l;
	CORBA_Environment ev;
	GNOME_C_Mngr m;
	guint i;
	GNOME_C_Mngr_DeviceList *dl;

	g_return_if_fail (GNOCAM_IS_CHOOSER (c));

	CORBA_exception_init (&ev);
	l = bonobo_activation_query ("repo_ids.has('IDL:GNOME/C/Mngr')", NULL,
				     &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not find any camera manager!");
		CORBA_exception_free (&ev);
		return;
	}

	for (i = 0; i < l->_length; i++) {
		m = bonobo_get_object (l->_buffer[i].iid, "IDL:GNOME/C/Mngr",
				       &ev);
		if (BONOB_EX (&ev))
			continue;
		dl = GNOME_C_Mngr_get_devices (m, &ev);
		bonobo_object_release_unref (m, NULL);
		if (BONOBO_EX (&ev))
			continue;
		CORBA_free (dl);
	}
	CORBA_free (l);
	CORBA_exception_free (&ev);
}

static void
on_cancel_clicked (GtkButton *button, GnocamChooser *c)
{
	g_warning ("Restore settings!");
	gtk_object_destroy (GTK_OBJECT (c));
}

static void
on_apply_clicked (GtkButton *button, GnocamChooser *c)
{
	g_warning ("Emit signals!");
}

static void
on_close_clicked (GtkButton *button, GnocamChooser *c)
{
	gtk_object_destroy (GTK_OBJECT (c));
}

GnocamChooser *
gnocam_chooser_new (void)
{
	GnocamChooser *c = g_object_new (GNOCAM_TYPE_CHOOSER, NULL);
	GladeXML *xml;
	GtkWidget *ui, *b;

	/* Read the interface description */
	xml = glade_xml_new (GNOCAM_GLADE_DIR "/gnocam.glade",
			     "hbox_contents", NULL);
	if (!xml) xml = glade_xml_new (GNOCAM_SRCDIR "/libgnocam/gnocam.glade",
				       "hbox_contents", NULL);
	if (!xml) return NULL;
	ui = glade_xml_get_widget (xml, "hbox_contents");
	g_object_unref (xml);
	if (!ui) return NULL;

	/* Create the contents */
	gtk_widget_show (ui);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (c)->vbox), ui, TRUE, TRUE, 0);

	/* Add the buttons */
	b = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
	gtk_widget_show (b);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (c)->action_area), b,
			    TRUE, TRUE, 0);
	g_signal_connect (b, "clicked", G_CALLBACK (on_cancel_clicked), c);
	b = gtk_button_new_from_stock (GTK_STOCK_APPLY);
	gtk_widget_show (b);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (c)->action_area), b, 
			    TRUE, TRUE, 0);
	g_signal_connect (b, "clicked", G_CALLBACK (on_apply_clicked), c);
	b = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_widget_show (b);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (c)->action_area), b,
			    TRUE, TRUE, 0);
	g_signal_connect (b, "clicked", G_CALLBACK (on_close_clicked), c);

	/* Find available cameras on the system. */
	gnocam_chooser_setup (c);

	return c;
}
