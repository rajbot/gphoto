#include <config.h>
#include "gnocam-capplet.h"

#include <string.h>

#include <gtk/gtkimage.h>
#include <gtk/gtkhbox.h>

#define PARENT_TYPE GTK_TYPE_DIALOG
static GtkDialogClass *parent_class = NULL;

struct _GnoCamCappletPrivate
{
	GConfClient *client;
};

static void
gnocam_capplet_destroy (GtkObject *object)
{
	GnoCamCapplet *capplet = GNOCAM_CAPPLET (object);

	if (capplet->priv->client) {
		gtk_object_unref (GTK_OBJECT (capplet->priv->client));
		capplet->priv->client = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_capplet_finalize (GObject *object)
{
	GnoCamCapplet *capplet = GNOCAM_CAPPLET (object);

	g_free (capplet->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_capplet_class_init (gpointer g_class, gpointer class_data)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gnocam_capplet_finalize;

	object_class = GTK_OBJECT_CLASS (g_class);
	object_class->destroy  = gnocam_capplet_destroy;

	parent_class = g_type_class_peek_parent (g_class);
}

static void
gnocam_capplet_init (GTypeInstance *instance, gpointer g_class)
{
	GnoCamCapplet *capplet = GNOCAM_CAPPLET (instance);

	capplet->priv = g_new0 (GnoCamCappletPrivate, 1);
}

GType
gnocam_capplet_get_type (void)
{
        static GType type = 0;

        if (!type) {
                GTypeInfo ti;

                memset (&ti, 0, sizeof (GTypeInfo));
                ti.class_size    = sizeof (GnoCamCappletClass);
                ti.class_init    = gnocam_capplet_class_init;
                ti.instance_size = sizeof (GnoCamCapplet);
                ti.instance_init = gnocam_capplet_init;

                type = g_type_register_static (PARENT_TYPE, "GnoCamCapplet",
                                               &ti, 0);
        }

        return (type);
}


GtkWidget*
gnocam_capplet_new (GConfClient *client)
{
	GnoCamCapplet *capplet;
	GtkWidget *hbox, *image;

	capplet = gtk_type_new (GNOCAM_TYPE_CAPPLET);
	
	/* Create a hbox */
	hbox = gtk_hbox_new (FALSE, 10);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (capplet)->vbox), hbox,
			    FALSE, FALSE, 10);

	/* Create the logo */
	image = gtk_image_new_from_file (IMAGEDIR "/gnocam-camera2.png");
	gtk_widget_show (image);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 10);

	return (GTK_WIDGET (capplet));
}
