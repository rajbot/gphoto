#include "config.h"
#include <gphoto2.h>
#include "gnocam-capplet-content.h"

#include <gal/util/e-util.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>
#include <gconf/gconf-client.h>

#include <gnocam-capplet-table-scrolled.h>

#define PARENT_TYPE GTK_TYPE_VBOX
static GtkVBoxClass *parent_class = NULL;

struct _GnoCamCappletContentPrivate {
	GtkWidget *table;

	GConfClient *client;
};

void
gnocam_capplet_content_ok (GnoCamCappletContent *content)
{
	gnocam_capplet_table_scrolled_ok (
			GNOCAM_CAPPLET_TABLE_SCROLLED (content->priv->table));
}

void
gnocam_capplet_content_revert (GnoCamCappletContent *content)
{
	gnocam_capplet_table_scrolled_revert (
			GNOCAM_CAPPLET_TABLE_SCROLLED (content->priv->table));
}

void
gnocam_capplet_content_try (GnoCamCappletContent *content)
{
	gnocam_capplet_table_scrolled_try (
			GNOCAM_CAPPLET_TABLE_SCROLLED (content->priv->table));
}

void
gnocam_capplet_content_cancel (GnoCamCappletContent *content)
{
	gnocam_capplet_table_scrolled_cancel (
			GNOCAM_CAPPLET_TABLE_SCROLLED (content->priv->table));
}

static void
gnocam_capplet_content_destroy (GtkObject *object)
{
	GnoCamCappletContent *content = GNOCAM_CAPPLET_CONTENT (object);

	if (content->priv->client) {
		gtk_object_unref (GTK_OBJECT (content->priv->client));
		content->priv->client = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_capplet_content_finalize (GtkObject *object)
{
	GnoCamCappletContent *content = GNOCAM_CAPPLET_CONTENT (object);

	g_free (content->priv);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_capplet_content_class_init (GnoCamCappletContentClass *klass)
{
	GtkObjectClass *object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy  = gnocam_capplet_content_destroy;
	object_class->finalize = gnocam_capplet_content_finalize;

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_capplet_content_init (GnoCamCappletContent* content)
{
	content->priv = g_new0 (GnoCamCappletContentPrivate, 1);
	content->priv->client = gconf_client_get_default ();
}

static void
on_autodetect_toggled (GtkToggleButton *toggle,
		       GnoCamCappletContent *content)
{
	gconf_client_set_bool (content->priv->client,
		"/apps/" PACKAGE "/autodetect", toggle->active, NULL);
}

GtkWidget*
gnocam_capplet_content_new (CappletWidget* capplet)
{
	GdkPixbuf *pixbuf;
	GdkPixmap *pixmap = NULL;
	GdkBitmap *bitmap = NULL;
	GnoCamCappletContent *new;
	GtkWidget *widget, *hbox, *check;

	new = gtk_type_new (GNOCAM_TYPE_CAPPLET_CONTENT);
	
	/* Create a hbox */
	hbox = gtk_hbox_new (FALSE, 10);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (new), hbox, FALSE, FALSE, 10);

	/* Create the logo */
	pixbuf = gdk_pixbuf_new_from_file (IMAGEDIR "/gnocam-camera2.png");
	gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap, 4);
	widget = gtk_pixmap_new (pixmap, bitmap);
	gtk_widget_show (widget);
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 10);

	/* Create the table */
	new->priv->table = gnocam_capplet_table_scrolled_new (capplet);
	gtk_widget_show (new->priv->table);
	gtk_box_pack_start (GTK_BOX (new), new->priv->table, TRUE, TRUE, 10);

	/* Create another hbox */
	hbox = gtk_hbox_new (FALSE, 10);
	gtk_widget_show (hbox);
	gtk_box_pack_end (GTK_BOX (new), hbox, FALSE, FALSE, 10);

	/* The "Do not ask again" check button */
	check = gtk_check_button_new_with_label (_("Always autodetect camera"));
	gtk_widget_show (check);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check),
		gconf_client_get_bool (new->priv->client,
			"/apps/" PACKAGE "/autodetect", NULL));
	gtk_container_add (GTK_CONTAINER (hbox), check);
	gtk_signal_connect (GTK_OBJECT (check), "toggled",
			    GTK_SIGNAL_FUNC (on_autodetect_toggled), new);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_capplet_content, "GnoCamCappletContent", GnoCamCappletContent, gnocam_capplet_content_class_init, gnocam_capplet_content_init, PARENT_TYPE)

