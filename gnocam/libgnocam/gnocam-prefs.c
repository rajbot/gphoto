#include <config.h>
#include "gnocam-prefs.h"

#include <gtk/gtktreeview.h>
#include <gtk/gtktreestore.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkcellrendererpixbuf.h>

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-exception.h>

enum {
	COL_PIXBUF = 0,
	COL_NAME,
	COL_ID,
	NUM_COLS
};

struct _GnocamPrefsPriv {
	GNOME_C_Bag bag;

	GtkTreeStore *s;
};

#define PARENT_TYPE GTK_TYPE_HPANED
static GtkHPanedClass *parent_class;

static void
gnocam_prefs_finalize (GObject *o)
{
	GnocamPrefs *p = GNOCAM_PREFS (o);

	bonobo_object_release_unref (p->priv->bag, NULL);
	g_free (p->priv);

	G_OBJECT_CLASS (parent_class)->finalize (o);
}

static void
gnocam_prefs_class_init (gpointer g_class, gpointer class_data)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gnocam_prefs_finalize;

	parent_class = g_type_class_peek_parent (g_class);
}

static void
gnocam_prefs_init (GTypeInstance *instance, gpointer g_class)
{
	GnocamPrefs *p = GNOCAM_PREFS (instance);

	p->priv = g_new0 (GnocamPrefsPriv, 1);
	p->priv->s = gtk_tree_store_new (NUM_COLS, GDK_TYPE_PIXBUF,
					 G_TYPE_STRING, G_TYPE_UINT);
}

GType
gnocam_prefs_get_type (void)
{
	static GType t = 0;

	if (!t) {
		static const GTypeInfo ti = {
			sizeof (GnocamPrefsClass), NULL, NULL,
			gnocam_prefs_class_init, NULL, NULL,
			sizeof (GnocamPrefs), 0, gnocam_prefs_init
		};
		t = g_type_register_static (PARENT_TYPE, "GnocamPrefs", &ti, 0);
	}
	return t;
}

static void
gnocam_prefs_add_bag (GnocamPrefs *p, GtkTreeIter *parent,
		      GNOME_C_Bag bag, CORBA_Environment *ev)
{
	GNOME_C_Bag_BagList *l;
	guint i;
	GtkTreeIter iter;
	CORBA_string name;
	GNOME_C_Icon *icon;
	GdkPixbufLoader *loader;

	g_return_if_fail (GNOCAM_IS_PREFS (p));

	l = GNOME_C_Bag__get_bags (bag, ev);
	if (BONOBO_EX (ev)) {CORBA_free (l); return;}
	for (i = 0; i < l->_length; i++) {
		gtk_tree_store_append (p->priv->s, &iter, parent);
		gtk_tree_store_set (p->priv->s, &iter, COL_ID, l, -1);
		
		name = GNOME_C_Bag__get_name (l->_buffer[i], ev);
		if (BONOBO_EX (ev)) {CORBA_free (l); return;}
		gtk_tree_store_set (p->priv->s, &iter, COL_NAME, name, -1);
		CORBA_free (name);
		
		icon = GNOME_C_Bag__get_icon (l->_buffer[i], ev);
		if (BONOBO_EX (ev)) {CORBA_free (l); return;}
		loader = gdk_pixbuf_loader_new ();
		gdk_pixbuf_loader_write (loader, icon->_buffer,
					 icon->_length, NULL);
		gdk_pixbuf_loader_close (loader, NULL);
		gtk_tree_store_set (p->priv->s, &iter, COL_PIXBUF,
				    gdk_pixbuf_loader_get_pixbuf (loader), -1);
		g_object_unref (loader);

		gnocam_prefs_add_bag (p, &iter, l->_buffer[i], ev);
		if (BONOBO_EX (ev)) {CORBA_free (l); return;}
	}
}

static void
on_row_activated (GtkTreeView *v, GtkTreePath *path, GtkTreeViewColumn *col,
		  GnocamPrefs *p)
{
	g_return_if_fail (GNOCAM_IS_PREFS (p));

	g_warning ("Fixme!");
}

GnocamPrefs *
gnocam_prefs_new (GNOME_C_Bag bag, CORBA_Environment *ev)
{
	GnocamPrefs *p = g_object_new (GNOCAM_TYPE_PREFS, NULL);
	GtkWidget *w;
	GtkTreeViewColumn *c;
	GtkCellRenderer *r;

	p->priv->bag = bonobo_object_dup_ref (bag, NULL);
	w = gtk_tree_view_new ();
	gtk_widget_show (w);
	gtk_paned_pack1 (GTK_PANED (p), w, TRUE, FALSE);
	gtk_tree_view_set_model (GTK_TREE_VIEW (w),
				 GTK_TREE_MODEL (p->priv->s));
	c = gtk_tree_view_column_new ();
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), c);
	r = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (c, r, FALSE);
	gtk_tree_view_column_add_attribute (c, r, "pixbuf", COL_PIXBUF);
	c = gtk_tree_view_column_new ();
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), c);
	r = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (c, r, TRUE);
	gtk_tree_view_column_add_attribute (c, r, "text", COL_NAME);
	g_signal_connect (w, "row_activated",
			  G_CALLBACK (on_row_activated), p);

	gnocam_prefs_add_bag (p, NULL, bag, ev);
	if (BONOBO_EX (ev)) {
		gtk_object_destroy (GTK_OBJECT (p));
		return NULL;
	}

	return p;
}
