#include "config.h"
#include "gnocam-capplet.h"

#include <gnocam/i18n.h>

#include <string.h>

#include <gtk/gtkimage.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkvbbox.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkcheckbutton.h>

#include <gdk/gdkkeysyms.h>

#include <libgnocam/gnocam-chooser.h>

enum
{
	COL_NAME = 0,
	COL_ID,
	COL_IS_EDITABLE,
	NUM_COLS
};

#define PARENT_TYPE GTK_TYPE_DIALOG
static GtkDialogClass *parent_class = NULL;

struct _GnocamCappletPrivate
{
	GConfClient *c;

	GtkTreeView *tv;
	GtkListStore *s;

	GHashTable *h;
};

static void
gnocam_capplet_destroy (GtkObject *object)
{
	GnocamCapplet *capplet = GNOCAM_CAPPLET (object);

	if (capplet->priv->c) {
		g_object_unref (capplet->priv->c);
		capplet->priv->c = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_capplet_finalize (GObject *object)
{
	GnocamCapplet *capplet = GNOCAM_CAPPLET (object);

	g_hash_table_destroy (capplet->priv->h);
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
	GnocamCapplet *c = GNOCAM_CAPPLET (instance);

	c->priv = g_new0 (GnocamCappletPrivate, 1);
	c->priv->s = gtk_list_store_new (NUM_COLS, G_TYPE_STRING,
					 G_TYPE_STRING, G_TYPE_BOOLEAN);
	c->priv->h = g_hash_table_new (g_str_hash, g_str_equal);
}

GType
gnocam_capplet_get_type (void)
{
        static GType type = 0;

        if (!type) {
                GTypeInfo ti;

                memset (&ti, 0, sizeof (GTypeInfo));
                ti.class_size    = sizeof (GnocamCappletClass);
                ti.class_init    = gnocam_capplet_class_init;
                ti.instance_size = sizeof (GnocamCapplet);
                ti.instance_init = gnocam_capplet_init;

                type = g_type_register_static (PARENT_TYPE, "GnocamCapplet",
                                               &ti, 0);
        }

        return (type);
}

static gboolean
gnocam_capplet_get_iter (GnocamCapplet *c, GtkTreeIter *iter,
			 const gchar *id)
{
	GtkTreeModel *m;
	GValue v = {0,};
	gboolean b;

	g_return_val_if_fail (GNOCAM_IS_CAPPLET (c), FALSE);
	g_return_val_if_fail (id != NULL, FALSE);

	m = GTK_TREE_MODEL (c->priv->s);
	b = gtk_tree_model_get_iter_first (m, iter);
	while (b) {
		gtk_tree_model_get_value (m, iter, COL_ID, &v);
		g_assert (g_value_get_string (&v));
		if (!strcmp (id, g_value_get_string (&v))) {
			g_value_unset (&v); return TRUE;
		}
		g_value_unset (&v); 
		b = gtk_tree_model_iter_next (m, iter);
	}
	return FALSE;
}

static void
on_close_clicked (GtkButton *button, GnocamCapplet *capplet)
{
	gtk_widget_destroy (GTK_WIDGET (capplet));
}

static void
on_new_clicked (GtkButton *button, GnocamCapplet *c)
{
	gchar *name = NULL;
	GSList *l;
	GError *e = NULL;
	guint i, j;
	gchar *k, *ek = NULL;

	g_return_if_fail (GNOCAM_IS_CAPPLET (c));

	/* Ask gconf for existing directories */
	l = gconf_client_all_dirs (c->priv->c, "/desktop/gnome/cameras", &e);
	if (e) {
		g_warning ("Could not read settings: %s", e->message);
		g_error_free (e);
		return;
	}

	/* Decide about a name for the new camera. */
	for (i = 1; i++; ) {
		name = g_strdup_printf (_("Camera %i"), i);
		ek = gconf_escape_key (name, strlen (name));
		for (j = 0; j < g_slist_length (l); j++)
			if (!strcmp (ek, g_basename (
				(gchar *) g_slist_nth_data (l, j)))) break;
		if (j == g_slist_length (l)) break;
		g_free (name);
		g_free (ek);
	}
	g_slist_foreach (l, (GFunc) g_free, NULL); g_slist_free (l);

	/* Tell gconf about the new camera. */
	k = g_strdup_printf ("/desktop/gnome/cameras/%s/name", ek);
	g_free (ek);
	gconf_client_set_string (c->priv->c, k, name, NULL);
	g_free (k);
	g_free (name);
	gconf_client_suggest_sync (c->priv->c, NULL);
}

static void
notify_func (GConfClient *client, guint cnxn_id, GConfEntry *entry,
	     gpointer user_data)
{
	GnocamCapplet *c = GNOCAM_CAPPLET (user_data);
	gchar *name, *id, *t;
	GtkTreeIter iter;
	const gchar *s;
	GnocamChooser *ch;

	if (!strcmp (entry->key, "/desktop/gnome/cameras")) return;

	/* Which setting changed? */
	s = g_basename (gconf_entry_get_key (entry));
	if (!entry->value) {
		if (!gnocam_capplet_get_iter (c, &iter, s)) return;
		gtk_list_store_remove (c->priv->s, &iter);
		ch = g_hash_table_lookup (c->priv->h, s);
		if (ch) gtk_object_destroy (GTK_OBJECT (ch));
	} else {
		id = g_strdup (entry->key);
		id[strlen (id) - strlen (g_basename (id)) - 1] = '\0';
		if (!gnocam_capplet_get_iter (c, &iter, id)) {
			gtk_list_store_append (c->priv->s, &iter);
			gtk_list_store_set (c->priv->s, &iter, COL_ID, id,
					    COL_IS_EDITABLE, TRUE, -1);
		}
		if (!strcmp (s, "name")) {
			name = gconf_client_get_string (c->priv->c,
					entry->key, NULL);
			gtk_list_store_set (c->priv->s, &iter,
					    COL_NAME, name, -1);
			g_free (name); g_free (id);
			gtk_tree_view_columns_autosize (c->priv->tv);
			return;
		}
		ch = g_hash_table_lookup (c->priv->h, id);
		g_free (id);
		if (!ch) return;
		t = gconf_client_get_string (c->priv->c, entry->key, NULL);
		if (!strcmp (s, "manufacturer"))
			gnocam_chooser_set_manufacturer (ch, t);
		else if (!strcmp (s, "model"))
			gnocam_chooser_set_model (ch, t);
		else if (!strcmp (s, "port"))
			gnocam_chooser_set_port (ch, t);
		g_free (t);
	}
}

static void
gnocam_capplet_delete (GnocamCapplet *c, GtkTreeIter *iter)
{
	GValue v = {0, };
	gchar *key, *id;
	GtkObject *o;
	
	g_return_if_fail (GNOCAM_IS_CAPPLET (c));

	gtk_tree_model_get_value (GTK_TREE_MODEL (c->priv->s), iter,
				  COL_ID, &v);
	gtk_list_store_remove (c->priv->s, iter);
	g_assert (g_value_get_string (&v));
	id = g_strdup (g_value_get_string (&v));
	g_value_unset (&v);
	o = g_hash_table_lookup (c->priv->h, id);
	if (o) {
		g_hash_table_remove (c->priv->h, id);
		gtk_object_destroy (o);
	}
	key = g_strdup_printf ("%s/name", id);
	gconf_client_unset (c->priv->c, key, NULL); g_free (key);
	key = g_strdup_printf ("%s/manufacturer", id);
	gconf_client_unset (c->priv->c, key, NULL); g_free (key);
	key = g_strdup_printf ("%s/model", id);
	gconf_client_unset (c->priv->c, key, NULL); g_free (key);
	key = g_strdup_printf ("%s/port", id);
	gconf_client_unset (c->priv->c, key, NULL); g_free (key);
	gconf_client_unset (c->priv->c, id, NULL);
	g_free (id);
	gconf_client_suggest_sync (c->priv->c, NULL);
}

static void
on_changed (GnocamChooser *ch, GnocamCapplet *c)
{
	gchar *key;
	const gchar *id, *s;

	g_return_if_fail (GNOCAM_IS_CAPPLET (c));
	g_return_if_fail (GNOCAM_IS_CHOOSER (ch));

	id = g_object_get_data (G_OBJECT (ch), "id");
	g_assert (id);

	s = gnocam_chooser_get_manufacturer (ch);
	if (s) {
		key = g_strdup_printf ("%s/manufacturer", id);
		gconf_client_set_string (c->priv->c, key, s, NULL);
		g_free (key);
	}
	s = gnocam_chooser_get_model (ch);
	if (s) {
		key = g_strdup_printf ("%s/model", id);
		gconf_client_set_string (c->priv->c, key, s, NULL);
		g_free (key);
	}
	s = gnocam_chooser_get_port (ch);
	if (s) {
		key = g_strdup_printf ("%s/port", id);
		gconf_client_set_string (c->priv->c, key, s, NULL);
		g_free (key);
	}
}

static void
on_destroy (GtkObject *o, GnocamCapplet *c)
{
	const gchar *id;

	g_return_if_fail (GNOCAM_IS_CAPPLET (c));

	id = g_object_get_data (G_OBJECT (o), "id");
	g_assert (id);
	g_hash_table_remove (c->priv->h, id);
}

static void
gnocam_capplet_edit (GnocamCapplet *c, GtkTreeIter *iter)
{
	GValue v = {0, };
	GtkWidget *d;
	GnocamChooser *ch;
	gchar *key, *id;

	g_return_if_fail (GNOCAM_IS_CAPPLET (c));

	gtk_tree_model_get_value (GTK_TREE_MODEL (c->priv->s), iter,
				  COL_ID, &v);
	g_assert (g_value_get_string (&v));
	id = g_strdup (g_value_get_string (&v));
	g_value_unset (&v);
	d = g_hash_table_lookup (c->priv->h, id);
	if (d) {gtk_window_present (GTK_WINDOW (d)); g_free (id); return;}

	gtk_widget_show (GTK_WIDGET (ch = gnocam_chooser_new ()));
	g_signal_connect (ch, "changed", G_CALLBACK (on_changed), c);
	g_signal_connect (ch, "destroy", G_CALLBACK (on_destroy), c);
	g_object_set_data_full (G_OBJECT (ch), "id", id,
				(GDestroyNotify) g_free);
	g_hash_table_insert (c->priv->h, g_strdup (id), ch);
	key = g_strdup_printf ("%s/manufacturer", id);
	gnocam_chooser_set_manufacturer (ch,
		gconf_client_get_string (c->priv->c, key, NULL));
	g_free (key);
	key = g_strdup_printf ("%s/model", id);
	gnocam_chooser_set_model (ch,
		 gconf_client_get_string (c->priv->c, key, NULL));
	g_free (key);
	key = g_strdup_printf ("%s/port", id);
	gnocam_chooser_set_port (ch,
		 gconf_client_get_string (c->priv->c, key, NULL));
	g_free (key);
}

static void
on_name_edited (GtkCellRendererText *cell, const gchar *path,
		const gchar *new_name, GnocamCapplet *c)
{
	GtkTreeIter iter;
	GValue v = {0, };
	gchar *k;
	gchar *old_name;

	g_return_if_fail (GNOCAM_IS_CAPPLET (c));

	/* Changed at all? */
	gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (c->priv->s),
					     &iter, path);
	gtk_tree_model_get_value (GTK_TREE_MODEL (c->priv->s), &iter,
				  COL_NAME, &v);
	old_name = g_strdup (g_value_get_string (&v));
	g_value_unset (&v);
	if (old_name && !strcmp (old_name, new_name)) {
		g_free (old_name);
		return;
	}
	g_free (old_name);

	gtk_tree_model_get_value (GTK_TREE_MODEL (c->priv->s), &iter,
				  COL_ID, &v);
	k = g_strdup_printf ("%s/name", g_value_get_string (&v));
	g_value_unset (&v);
	gconf_client_set_string (c->priv->c, k, new_name, NULL);
	g_free (k);
}

static void
edit_foreach_func (GtkTreeModel *m, GtkTreePath *p, GtkTreeIter *iter,
		   gpointer data)
{
	gnocam_capplet_edit (GNOCAM_CAPPLET (data), iter);
}

static void
gnocam_capplet_delete_selected (GnocamCapplet *c)
{
	GList *l, *lr = NULL;
	guint i;
	GtkTreeIter iter;
	GtkTreeModel *m = NULL;
	GtkTreePath *p;

	g_return_if_fail (GNOCAM_IS_CAPPLET (c));

	l = gtk_tree_selection_get_selected_rows (
		gtk_tree_view_get_selection (c->priv->tv), &m);
	for (i = 0; i < g_list_length (l); i++)
		lr = g_list_append (lr, gtk_tree_row_reference_new (m,
			(GtkTreePath *) g_list_nth_data (l, i)));
	g_list_foreach (l, (GFunc) gtk_tree_path_free, NULL); g_list_free (l);
	for (i = 0; i < g_list_length (lr); i++) {
		p = gtk_tree_row_reference_get_path ((GtkTreeRowReference *)
			g_list_nth_data (lr, i));
		gtk_tree_model_get_iter (m, &iter, p);
		gnocam_capplet_delete (c, &iter);
	}
	g_list_foreach (lr, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_free (lr);
}

static void
gnocam_capplet_edit_selected (GnocamCapplet *c)
{
	g_return_if_fail (GNOCAM_IS_CAPPLET (c));

	gtk_tree_selection_selected_foreach (
		gtk_tree_view_get_selection (c->priv->tv),
		edit_foreach_func, c);
}

static gboolean
on_tree_view_key_press_event (GtkWidget *widget, GdkEventKey *event,
			      GnocamCapplet *c)
{
	switch (event->keyval) {
	case GDK_Delete:
		gnocam_capplet_delete_selected (c);
		return TRUE;
	default:
		return FALSE;
	}
}

static gboolean
on_tree_view_button_press_event (GtkWidget *widget, GdkEventButton *event,
				 GnocamCapplet *capplet)
{
	if (event->button == 3) {
		g_warning ("Implement popup!");
		return (TRUE);
	}

	return (FALSE);
}

static void
gnocam_capplet_load (GnocamCapplet *c)
{
	GtkTreeIter iter;
	GSList *l;
	GError *e = NULL;
	guint i;
	gchar *name;
	gchar *k;

	g_return_if_fail (GNOCAM_IS_CAPPLET (c));

	l = gconf_client_all_dirs (c->priv->c, "/desktop/gnome/cameras", &e);
	if (e) {
		g_warning ("Could not read settings: %s", e->message);
		return;
	}
	gtk_list_store_clear (c->priv->s);
	for (i = 0; i < g_slist_length (l); i++) {
		gtk_list_store_append (c->priv->s, &iter);
		k = g_strdup_printf ("%s/name",
			(gchar *) g_slist_nth_data (l, i));
		name = gconf_client_get_string (c->priv->c, k, NULL);
		g_free (k);
		gtk_list_store_set (c->priv->s, &iter, COL_NAME, name,
			COL_ID, g_slist_nth_data (l, i),
			COL_IS_EDITABLE, TRUE, -1);
		g_free (name);
		if (!i) gtk_tree_selection_select_iter (
				gtk_tree_view_get_selection (c->priv->tv),
				&iter);
	}
	g_slist_foreach (l, (GFunc) g_free, NULL); g_slist_free (l);
	gtk_tree_view_columns_autosize (c->priv->tv);
}

static void
on_edit_clicked (GtkButton *button, GnocamCapplet *c)
{
	gnocam_capplet_edit_selected (c);
}

static void
on_remove_clicked (GtkButton *button, GnocamCapplet *c)
{
	gnocam_capplet_delete_selected (c);
}

GtkWidget*
gnocam_capplet_new (GConfClient *client)
{
	GnocamCapplet *c = gtk_type_new (GNOCAM_TYPE_CAPPLET);
	GtkWidget *hbox, *image, *w, *b, *bbox;
	GtkCellRenderer *r;
	GtkTreeViewColumn *col;

	/* Watch out for changes */
	c->priv->c = client;
	g_object_ref (client);
	gconf_client_notify_add (client, "/desktop/gnome/cameras", notify_func,
				 c, NULL, NULL);

	/* Create a hbox */
	gtk_widget_show (hbox = gtk_hbox_new (FALSE, 5));
	gtk_box_set_spacing (GTK_BOX (hbox), 5);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (c)->vbox), hbox,
			    TRUE, TRUE, 10);

	/* Create the logo */
	image = gtk_image_new_from_file (IMAGEDIR "/gnocam-camera2.png");
	gtk_widget_show (image);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

	/* Create the buttons */
	gtk_widget_show (bbox = gtk_vbutton_box_new ());
	gtk_box_set_spacing (GTK_BOX (bbox), 5);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_START);
	gtk_box_pack_end (GTK_BOX (hbox), bbox, FALSE, FALSE, 0);
	gtk_widget_show (b = gtk_button_new_with_label (_("_New")));
	gtk_box_pack_start (GTK_BOX (bbox), b, FALSE, FALSE, 0);
	g_signal_connect (b, "clicked", G_CALLBACK (on_new_clicked), c);
	gtk_widget_show (b = gtk_button_new_with_label (_("_Edit")));
	gtk_box_pack_start (GTK_BOX (bbox), b, FALSE, FALSE, 0);
	g_signal_connect (b, "clicked", G_CALLBACK (on_edit_clicked), c);
	gtk_widget_show (b = gtk_button_new_with_label (_("_Remove")));
	gtk_box_pack_start (GTK_BOX (bbox), b, FALSE, FALSE, 0);
	g_signal_connect (b, "clicked", G_CALLBACK (on_remove_clicked), c);

	/* Create the table */
	gtk_widget_show (w = gtk_tree_view_new ());
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (w), FALSE);
	gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
	g_signal_connect (w, "button_press_event",
		G_CALLBACK (on_tree_view_button_press_event), c);
	g_signal_connect (w, "key_press_event",
		G_CALLBACK (on_tree_view_key_press_event), c);
	c->priv->tv = GTK_TREE_VIEW (w);
	gtk_tree_view_set_model (GTK_TREE_VIEW (w),
				 GTK_TREE_MODEL (c->priv->s));
	gtk_tree_selection_set_mode (gtk_tree_view_get_selection (c->priv->tv),
				     GTK_SELECTION_MULTIPLE);

	/* Add the column for the name */
	r = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Name"), r,
		"text", COL_NAME, "editable", COL_IS_EDITABLE, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), col);
	g_signal_connect (r, "edited", G_CALLBACK (on_name_edited), c);

	/* Add the close-button. */
	gtk_widget_show (b = gtk_button_new_from_stock (GTK_STOCK_CLOSE));
	gtk_box_pack_end (GTK_BOX (GTK_DIALOG (c)->action_area), b,
			  FALSE, FALSE, 0);
	g_signal_connect (b, "clicked", G_CALLBACK (on_close_clicked), c);

	/* Load the current settings. */
	gnocam_capplet_load (c);

	return (GTK_WIDGET (c));
}
