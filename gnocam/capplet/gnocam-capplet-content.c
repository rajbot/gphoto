#include <gphoto2.h>
#include "gnocam-capplet-content.h"

#include <gal/util/e-util.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <gnocam-capplet-table-scrolled.h>

#define PARENT_TYPE GTK_TYPE_VBOX
static GtkVBoxClass *parent_class = NULL;

struct _GnoCamCappletContentPrivate 
{
	GtkWidget *table;
};

/*****************/
/* Our functions */
/*****************/

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

/*************/
/* Gtk stuff */
/*************/

static void
gnocam_capplet_content_destroy (GtkObject *object)
{
	GnoCamCappletContent *content;

	content = GNOCAM_CAPPLET_CONTENT (object);

	g_free (content->priv);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_capplet_content_class_init (GnoCamCappletContentClass *klass)
{
	GtkObjectClass *object_class;

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
	GtkWidget*		widget;

	new = gtk_type_new (GNOCAM_TYPE_CAPPLET_CONTENT);
	
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

	/* Create the table */
	new->priv->table = gnocam_capplet_table_scrolled_new (capplet);
	gtk_widget_show (new->priv->table);
	gtk_box_pack_start (GTK_BOX (hbox), new->priv->table, TRUE, TRUE, 10);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_capplet_content, "GnoCamCappletContent", GnoCamCappletContent, gnocam_capplet_content_class_init, gnocam_capplet_content_init, PARENT_TYPE)

