#include "config.h"
#include "gnocam-capplet.h"
#include "i18n.h"

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

enum
{
	COL_NAME = 0,
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
					 G_TYPE_BOOLEAN);
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
			 const gchar *name)
{
	GtkTreeModel *m;
	GValue v = {0,};
	gboolean b;

	g_return_val_if_fail (GNOCAM_IS_CAPPLET (c), FALSE);
	g_return_val_if_fail (name != NULL, FALSE);

	m = GTK_TREE_MODEL (c->priv->s);
	b = gtk_tree_model_get_iter_first (m, iter);
	while (b) {
		gtk_tree_model_get_value (m, iter, COL_NAME, &v);
		if (!strcmp (name, g_value_get_string (&v))) {
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
		ek = gconf_escape_key (name, strlen (name)); g_free (name);
		for (j = 0; j < g_slist_length (l); j++)
			if (!strcmp (ek, g_basename (
				(gchar *) g_slist_nth_data (l, j)))) break;
		if (j == g_slist_length (l)) break;
		g_free (ek);
	}
	g_slist_foreach (l, (GFunc) g_free, NULL); g_slist_free (l);

	/* Tell gconf about the new camera. */
	k = g_strdup_printf ("/desktop/gnome/cameras/%s/manufacturer", ek);
	g_free (ek);
	gconf_client_set_string (c->priv->c, k, "", NULL); g_free (k);
	gconf_client_suggest_sync (c->priv->c, NULL);
}

static void
notify_func (GConfClient *client, guint cnxn_id, GConfEntry *entry,
	     gpointer user_data)
{
	GnocamCapplet *c = GNOCAM_CAPPLET (user_data);
	gchar *name, *k;
	GtkTreeIter iter;

	/* Which setting changed? */
	if (!entry->value) {
		name = gconf_unescape_key (
			g_basename (gconf_entry_get_key (entry)),
			strlen (g_basename (gconf_entry_get_key (entry))));
		if (!gnocam_capplet_get_iter (c, &iter, name)) {
			g_free (name); return;
		}
		gtk_list_store_remove (c->priv->s, &iter);
	} else {
		k = g_strdup (entry->key);
		k[strlen (k) - strlen (g_basename (k)) - 1] = '\0';
		name = gconf_unescape_key (g_basename (k),
					   strlen (g_basename (k)));
		g_free (k);
		gtk_list_store_append (c->priv->s, &iter);
		gtk_list_store_set (c->priv->s, &iter, COL_NAME, name, -1);
		g_free (name);
		gtk_tree_selection_select_iter (
			gtk_tree_view_get_selection (c->priv->tv), &iter);
	}
}

static void
gnocam_capplet_delete (GnocamCapplet *c, GtkTreeIter *iter)
{
	GValue v = {0, };
	gchar *ek, *k, *key;
	
	g_return_if_fail (GNOCAM_IS_CAPPLET (c));

	gtk_tree_model_get_value (GTK_TREE_MODEL (c->priv->s), iter,
				  COL_NAME, &v);
	gtk_list_store_remove (c->priv->s, iter);
	ek = gconf_escape_key (g_value_get_string (&v),
			       strlen (g_value_get_string (&v)));
	g_value_unset (&v);
	k = g_strdup_printf ("/desktop/gnome/cameras/%s", ek);
	g_free (ek);
	key = g_strdup_printf ("%s/manufacturer", k);
	gconf_client_unset (c->priv->c, key, NULL); g_free (key);
	key = g_strdup_printf ("%s/model", k);
	gconf_client_unset (c->priv->c, key, NULL); g_free (key);
	key = g_strdup_printf ("%s/port", k);
	gconf_client_unset (c->priv->c, key, NULL); g_free (key);
	gconf_client_unset (c->priv->c, k, NULL);
	g_free (k);
	gconf_client_suggest_sync (c->priv->c, NULL);
}

static void
gnocam_capplet_edit (GnocamCapplet *c, GtkTreeIter *iter)
{
	GValue v = {0, };

	g_return_if_fail (GNOCAM_IS_CAPPLET (c));

	gtk_tree_model_get_value (GTK_TREE_MODEL (c->priv->s), iter,
				  COL_NAME, &v);
	g_warning ("Implement editing of '%s'!", g_value_get_string (&v));
	g_value_unset (&v);
}

static void
on_name_edited (GtkCellRendererText *cell, const gchar *path,
		const gchar *new_text, GnocamCapplet *c)
{
	GtkTreeIter iter;
	GValue v = {0, };
	gchar *k, *key, *et;
	gchar *manuf, *model, *port;

	g_return_if_fail (GNOCAM_IS_CAPPLET (c));

	/* Changed at all? */
	gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (c->priv->s),
					     &iter, path);
	gtk_tree_model_get_value (GTK_TREE_MODEL (c->priv->s), &iter,
				  COL_NAME, &v);
	if (!strcmp (g_value_get_string (&v), new_text)) {
		g_value_unset (&v);
		return;
	}

	/* Make sure the name does not already exist. */
	k = g_strdup_printf ("/desktop/gnome/cameras/%s", new_text);
	if (gconf_client_dir_exists (c->priv->c, k, NULL)) {
		g_warning ("There is already a camera with the name '%s'!",
			   new_text);
		g_value_unset (&v);
		g_free (k);
		return;
	}
	g_free (k);

	/* Remove the old settings. */
	et = gconf_escape_key (g_value_get_string (&v),
			       strlen (g_value_get_string (&v)));
	g_value_unset (&v);
	k = g_strdup_printf ("/desktop/gnome/cameras/%s", et);
	key = g_strdup_printf ("%s/manufacturer", k);
	manuf = gconf_client_get_string (c->priv->c, key, NULL);
	gconf_client_unset (c->priv->c, key, NULL);
	g_free (key);
	key = g_strdup_printf ("%s/model", k);
	model = gconf_client_get_string (c->priv->c, key, NULL);
	gconf_client_unset (c->priv->c, key, NULL);
	g_free (key);
	key = g_strdup_printf ("%s/port", k);
	port = gconf_client_get_string (c->priv->c, key, NULL);
	gconf_client_unset (c->priv->c, key, NULL);
	g_free (key);
	gconf_client_unset (c->priv->c, k, NULL);
	g_free (k);

	/* Add the new settings */
	k = g_strdup_printf ("/desktop/gnome/cameras/%s", et);
	g_free (et);
	if (manuf) {
		key = g_strdup_printf ("%s/manufacturer", k);
		gconf_client_set_string (c->priv->c, k, manuf, NULL);
		g_free (key); g_free (manuf);
	}
	if (model) {
		key = g_strdup_printf ("%s/model", k);
		gconf_client_set_string (c->priv->c, k, model, NULL);
		g_free (key); g_free (model);
	}
	if (port) {
		key = g_strdup_printf ("%s/port", k);
		gconf_client_set_string (c->priv->c, k, port, NULL);
		g_free (key); g_free (port);
	}
	g_free (k);
	gconf_client_suggest_sync (c->priv->c, NULL);
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

	g_return_if_fail (GNOCAM_IS_CAPPLET (c));

	l = gconf_client_all_dirs (c->priv->c, "/desktop/gnome/cameras", &e);
	if (e) {
		g_warning ("Could not read settings: %s", e->message);
		return;
	}
	gtk_list_store_clear (c->priv->s);
	for (i = 0; i < g_slist_length (l); i++) {
		gtk_list_store_append (c->priv->s, &iter);
		name = gconf_unescape_key (
			g_basename (g_slist_nth_data (l, i)),
			strlen (g_basename (g_slist_nth_data (l, i))));
		gtk_list_store_set (c->priv->s, &iter, COL_NAME, name,
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
