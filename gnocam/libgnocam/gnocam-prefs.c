#include <config.h>
#include "gnocam-prefs.h"

#include <string.h>

#include <gtk/gtktreeview.h>
#include <gtk/gtktreestore.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkentry.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkcellrendererpixbuf.h>

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-exception.h>

enum {
	COL_PIXBUF = 0,
	COL_NAME,
	COL_BAG,
	NUM_COLS
};

struct _GnocamPrefsPriv {
	GNOME_C_Bag bag;

	GList *bags;

	GtkTreeStore *s;
	GtkWidget *vbox;
};

#define PARENT_TYPE GTK_TYPE_HPANED
static GtkHPanedClass *parent_class;

static void
gnocam_prefs_finalize (GObject *o)
{
	GnocamPrefs *p = GNOCAM_PREFS (o);
	guint i;

	for (i = 0; i < g_list_length (p->priv->bags); i++)
		bonobo_object_release_unref (
				g_list_nth_data (p->priv->bags, i), NULL);
	g_list_free (p->priv->bags);
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
	GtkWidget *s, *v;

	p->priv = g_new0 (GnocamPrefsPriv, 1);
	p->priv->s = gtk_tree_store_new (NUM_COLS, GDK_TYPE_PIXBUF,
					 G_TYPE_STRING, G_TYPE_POINTER);
	gtk_widget_show (s = gtk_scrolled_window_new (NULL, NULL));
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (s),
					     GTK_SHADOW_NONE);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (s),
				GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_set_border_width (GTK_CONTAINER (s), 2);
	gtk_paned_pack2 (GTK_PANED (p), s, TRUE, FALSE);
	gtk_widget_show (v = gtk_viewport_new (NULL, NULL));
	gtk_container_set_border_width (GTK_CONTAINER (v), 3);
	gtk_container_add (GTK_CONTAINER (s), v);
	gtk_widget_show (p->priv->vbox = gtk_vbox_new (FALSE, 5));
	gtk_container_add (GTK_CONTAINER (v), p->priv->vbox);
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
on_entry_changed (GtkEntry *entry, GnocamPrefs *p)
{
	GNOME_C_Prop prop = g_object_get_data (G_OBJECT (entry), "prop");
	CORBA_Environment ev;

	g_return_if_fail (GNOCAM_IS_PREFS (p));
	g_return_if_fail (GTK_IS_ENTRY (entry));

	CORBA_exception_init (&ev);
	GNOME_C_Prop_set_string (prop, gtk_entry_get_text (entry), &ev);
	if (BONOBO_EX (&ev))
		g_warning ("Could not set value: %s",
			   bonobo_exception_get_text (&ev));
	CORBA_exception_free (&ev);
}

static void
on_choice_changed (GtkEntry *entry, GnocamPrefs *p)
{
	GNOME_C_Prop prop = g_object_get_data (G_OBJECT (entry), "prop");
	GList *list = g_object_get_data (G_OBJECT (entry), "list");
	CORBA_Environment ev;
	CORBA_long l;

	for (l = 0; l < g_list_length (list); l++)
		if (!strcmp (g_list_nth_data (list, l),
			     gtk_entry_get_text (entry)))
			break;
	if (l == g_list_length (list)) {
		g_warning ("Could not find value in list!");
		return;
	}
	CORBA_exception_init (&ev);
	GNOME_C_Prop_set_long (prop, l, &ev);
	if (BONOBO_EX (&ev))
		g_warning ("Could not set value: %s",
			   bonobo_exception_get_text (&ev));
	CORBA_exception_free (&ev);
}

static void
unref_bonobo_object (gpointer data)
{
	bonobo_object_release_unref (data, NULL);
}

static void
unref_string_list (gpointer data)
{
	GList *list = data;
	guint i;

	for (i = 0; i < g_list_length (list); i++)
		g_free (g_list_nth_data (list, i));
	g_free (list);
}

static void
gnocam_prefs_display_bag (GnocamPrefs *p, GNOME_C_Bag bag,
			  CORBA_Environment *ev)
{
	GNOME_C_PropList *l;
	guint i, j;
	CORBA_string name;
	GNOME_C_Val *v;
	GtkWidget *w, *hbox;
	GList *list;

	l = GNOME_C_Bag__get_props (bag, ev);
	if (BONOBO_EX (ev)) return;

	/*
	 * Remove the currently displayed properties and set up the
	 * widget to contain the new properties.
	 */
	list = gtk_container_get_children (GTK_CONTAINER (p->priv->vbox));
	for (i = 0; i < g_list_length (list); i++)
		gtk_container_remove (GTK_CONTAINER (p->priv->vbox),
				      g_list_nth_data (list, i));
	g_free (list);
	for (i = 0; i < l->_length; i++) {

	    /* Name */
	    name = GNOME_C_Prop__get_name (l->_buffer[i], ev);
	    if (BONOBO_EX (ev)) {CORBA_free (l); return;}

	    /* Value */
	    v = GNOME_C_Prop__get_val (l->_buffer[i], ev);
	    if (BONOBO_EX (ev)) {CORBA_free (l); CORBA_free (name); return;}
	    switch (v->_d) {
	    case GNOME_C_VAL_TYPE_BOOLEAN:
		break;
	    case GNOME_C_VAL_TYPE_RANGE:
		break;
	    case GNOME_C_VAL_TYPE_STRING:
		gtk_widget_show (hbox = gtk_hbox_new (FALSE, 5));
		gtk_box_pack_start (GTK_BOX (p->priv->vbox), hbox,
				    FALSE, FALSE, 0);
		gtk_widget_show (w = gtk_label_new (name));
		gtk_box_pack_start (GTK_BOX (hbox), w, FALSE, FALSE, 0);
		gtk_widget_show (w = gtk_entry_new ());
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_entry_set_text (GTK_ENTRY (w), v->_u.p_string);
		g_signal_connect (w, "changed",
			G_CALLBACK (on_entry_changed), p);
		g_object_set_data_full (G_OBJECT (GTK_COMBO (w)->entry),
			"prop", bonobo_object_dup_ref (l->_buffer[i], NULL),
			unref_bonobo_object);
		
		break;
	    case GNOME_C_VAL_TYPE_DATE_TIME:
		break;
	    case GNOME_C_VAL_TYPE_CHOICE:
		gtk_widget_show (hbox = gtk_hbox_new (FALSE, 5));
		gtk_box_pack_start (GTK_BOX (p->priv->vbox), hbox,
				    FALSE, FALSE, 0);
		gtk_widget_show (w = gtk_label_new (name));
		gtk_box_pack_start (GTK_BOX (hbox), w, FALSE, FALSE, 0);
		gtk_widget_show (w = gtk_combo_new ());
		gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
		gtk_combo_set_value_in_list (GTK_COMBO (w), TRUE, TRUE);
		for (list = NULL, j = 0; j < v->_u.p_choice.vals._length; j++)
			list = g_list_append (list,
				g_strdup (v->_u.p_choice.vals._buffer[j]));
		gtk_combo_set_popdown_strings (GTK_COMBO (w), list);
		if ( v->_u.p_choice.val < v->_u.p_choice.vals._length)
			gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (w)->entry),
				g_list_nth_data (list,  v->_u.p_choice.val));
		else
			gtk_entry_set_text (
					GTK_ENTRY (GTK_COMBO (w)->entry), "");
		g_signal_connect (GTK_COMBO (w)->entry, "changed",
				  G_CALLBACK (on_choice_changed), p);
		g_object_set_data_full (G_OBJECT (GTK_COMBO (w)->entry),
			"prop", bonobo_object_dup_ref (l->_buffer[i], NULL),
			unref_bonobo_object);
		g_object_set_data_full (G_OBJECT (GTK_COMBO (w)->entry),
			"list", list, unref_string_list);
		break;
	    }
	    CORBA_free (v);
	    CORBA_free (name);
	}
}

static void
gnocam_prefs_add_bag (GnocamPrefs *p, GtkTreeIter *parent,
		      GNOME_C_Bag bag, CORBA_Environment *ev)
{
	GNOME_C_Bag_BagList *l;
	guint i;
	GtkTreeIter iter;
	CORBA_string name;
	GNOME_C_Bag c;
#if 0
	GNOME_C_Icon *icon;
	GdkPixbufLoader *loader;
#endif

	g_return_if_fail (GNOCAM_IS_PREFS (p));

	l = GNOME_C_Bag__get_bags (bag, ev);
	if (BONOBO_EX (ev)) {CORBA_free (l); return;}
	for (i = 0; i < l->_length; i++) {
		gtk_tree_store_append (p->priv->s, &iter, parent);

		/* Remember the bag. */
		c = bonobo_object_dup_ref (l->_buffer[i], NULL);
		p->priv->bags = g_list_append (p->priv->bags, c);
		gtk_tree_store_set (p->priv->s, &iter, COL_BAG, c, -1);

		/* Name */
		name = GNOME_C_Bag__get_name (l->_buffer[i], ev);
		if (BONOBO_EX (ev)) {CORBA_free (l); return;}
		gtk_tree_store_set (p->priv->s, &iter, COL_NAME, name, -1);
		CORBA_free (name);

#if 0
		/* Icon */
		icon = GNOME_C_Bag__get_icon (l->_buffer[i], ev);
		if (BONOBO_EX (ev)) {CORBA_free (l); return;}
		loader = gdk_pixbuf_loader_new ();
		if (!gdk_pixbuf_loader_write (loader, icon->_buffer,
					      icon->_length, NULL))
			g_warning ("Could not write data to loader!");
		CORBA_free (icon);
		gdk_pixbuf_loader_close (loader, NULL);
		if (!gdk_pixbuf_loader_get_pixbuf (loader))
			g_warning ("Could not get pixbuf!");
		gtk_tree_store_set (p->priv->s, &iter, COL_PIXBUF,
				    gdk_pixbuf_loader_get_pixbuf (loader), -1);
		g_object_unref (loader);
#endif

		gnocam_prefs_add_bag (p, &iter, l->_buffer[i], ev);
		if (BONOBO_EX (ev)) {CORBA_free (l); return;}
	}
	CORBA_free (l);
}

static void
on_row_activated (GtkTreeView *tv, GtkTreePath *path, GtkTreeViewColumn *col,
		  GnocamPrefs *p)
{
	GtkTreeIter iter;
	GValue v = {0,};
	GNOME_C_Bag bag;
	CORBA_Environment ev;

	g_return_if_fail (GNOCAM_IS_PREFS (p));

	if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (p->priv->s),
				      &iter, path)) {
		g_warning ("Could not get activated entry!");
		return;
	}
	gtk_tree_model_get_value (GTK_TREE_MODEL (p->priv->s), &iter,
				  COL_BAG, &v);
	bag = g_value_get_pointer (&v);
	g_value_unset (&v);

	CORBA_exception_init (&ev);
	gnocam_prefs_display_bag (p, bag, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not display properties: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return;
	}
	CORBA_exception_free (&ev);
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
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (w), FALSE);
	gtk_widget_show (w);
	gtk_paned_pack1 (GTK_PANED (p), w, TRUE, FALSE);
	gtk_tree_view_set_model (GTK_TREE_VIEW (w),
				 GTK_TREE_MODEL (p->priv->s));
	g_signal_connect (w, "row_activated", G_CALLBACK (on_row_activated), p);

	/* Column for icons */
	c = gtk_tree_view_column_new ();
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), c);
	r = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (c, r, TRUE);
	gtk_tree_view_column_add_attribute (c, r, "pixbuf", COL_PIXBUF);

	/* Column for names */
	c = gtk_tree_view_column_new ();
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), c);
	r = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (c, r, TRUE);
	gtk_tree_view_column_add_attribute (c, r, "text", COL_NAME);

	/* Build the tree. */
	p->priv->bags = g_list_append (p->priv->bags,
				       bonobo_object_dup_ref (bag, NULL));
	gnocam_prefs_add_bag (p, NULL, bag, ev);
	if (BONOBO_EX (ev)) {
		gtk_object_destroy (GTK_OBJECT (p));
		return NULL;
	}
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW (w));

	return p;
}
