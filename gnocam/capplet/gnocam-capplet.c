#include <config.h>
#include "gnocam-capplet.h"

#include <string.h>

#include <gtk/gtkimage.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkcellrenderertext.h>

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) gettext (String)
#else
#  define _(String) (String)
#endif

enum
{
	COL_NAME = 0,
	COL_MODEL,
	COL_PORT,
	NUM_COLS
};

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
	GtkWidget *hbox, *image, *vbox, *w;
	GtkListStore *s;
	GtkCellRenderer *c;
	GtkTreeViewColumn *col;

	capplet = gtk_type_new (GNOCAM_TYPE_CAPPLET);
	
	/* Create a hbox */
	hbox = gtk_hbox_new (FALSE, 10);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (capplet)->vbox), hbox,
			    FALSE, FALSE, 10);

	/* Create the logo */
	image = gtk_image_new_from_file (IMAGEDIR "/gnocam-camera2.png");
	gtk_widget_show (image);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

	/* Create a vbox */
	vbox = gtk_vbox_new (FALSE, 10);
	gtk_widget_show (vbox);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

	/* Create the table */
	w = gtk_tree_view_new ();
	gtk_widget_show (w);
	gtk_box_pack_start (GTK_BOX (vbox), w, TRUE, TRUE, 0);

	/* Create the model for the table */
	s = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_STRING,
				G_TYPE_STRING);
	gtk_tree_view_set_model (GTK_TREE_VIEW (w), GTK_TREE_MODEL (s));

	/* Add the columns */
	c = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Name"), c,
						"text", COL_NAME, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), col);
	c = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Model"), c,
						"text", COL_MODEL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), col);
	c = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Port"), c,
						"text", COL_PORT, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), col);

	return (GTK_WIDGET (capplet));
}
