#include "config.h"
#include "gnocam-capplet.h"
#include "i18n.h"

#include <string.h>

#include <gtk/gtkimage.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkcheckbutton.h>

#include <gdk/gdkkeysyms.h>

enum
{
	COL_NUMBER = 0,
	COL_NAME,
	COL_MODEL,
	COL_PORT,
	COL_IS_EDITABLE,
	NUM_COLS
};

#define PARENT_TYPE GTK_TYPE_DIALOG
static GtkDialogClass *parent_class = NULL;

struct _GnoCamCappletPrivate
{
	GConfClient *client;

	GtkTreeModel *model;

	GtkTreeIter last;
};

static void
gnocam_capplet_destroy (GtkObject *object)
{
	GnoCamCapplet *capplet = GNOCAM_CAPPLET (object);

	if (capplet->priv->client) {
		g_object_unref (capplet->priv->client);
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

static void
on_ok_clicked (GtkButton *button, GnoCamCapplet *capplet)
{
	gtk_main_quit ();
}

static void
gnocam_capplet_add_entry (GnoCamCapplet *capplet)
{
	guint n = 0;
	gchar *k = NULL;

	g_return_if_fail (GNOCAM_IS_CAPPLET (capplet));

	while (1) {
		k = g_strdup_printf ("/desktop/gnome/cameras/%i", n);
		if (!gconf_client_dir_exists (capplet->priv->client, k, NULL))
			break;
		g_free (k);
		n++;
	}
	g_free (k);

	gtk_list_store_append (GTK_LIST_STORE (capplet->priv->model),
			       &capplet->priv->last);
	gtk_list_store_set (GTK_LIST_STORE (capplet->priv->model),
		&capplet->priv->last, COL_IS_EDITABLE, TRUE,
		COL_NUMBER, n, -1);
}

static gboolean
gnocam_capplet_get_iter (GnoCamCapplet *capplet, guint n, GtkTreeIter *iter)
{
	GValue v = {0, };
	guint m;

	/* Search the camera in the list. */
	if (!gtk_tree_model_get_iter_first (capplet->priv->model, iter))
		return FALSE;
	while (1) {
		gtk_tree_model_get_value (capplet->priv->model, iter,
					  COL_NUMBER, &v);
		m = g_value_get_uint (&v);
		g_value_unset (&v);
		if (n == m)
			return TRUE;
		if (!gtk_tree_model_iter_next (capplet->priv->model, iter)) {
			return FALSE;
		}
	}
}

static void
notify_func (GConfClient *client, guint cnxn_id, GConfEntry *entry,
	     gpointer user_data)
{
	GnoCamCapplet *capplet = GNOCAM_CAPPLET (user_data);
	gchar *key;
	const gchar *s;
	guint n, m;
	guint c = 0;
	GValue v = {0, };
	GtkTreeIter iter;
	GConfValue *value;

	key = g_strdup (gconf_entry_get_key (entry));

	/* Which setting changed? */
	s = strrchr (key, '/');
	g_return_if_fail (s);
	if (!strcmp (s + 1, "name"))
		c = COL_NAME;
	else if (!strcmp (s + 1, "model"))
		c = COL_MODEL;
	else if (!strcmp (s + 1, "port"))
		c = COL_PORT;
	else if ((*(s + 1) >= '0') && (*(s + 1) <= '9')) {

		/* A camera got removed. */
		n = atoi (s + 1);
		g_free (key);
		if (!gnocam_capplet_get_iter (capplet, n, &iter)) {
			g_warning ("Could not find camera %i in list!", n);
			return;
		}
		gtk_list_store_remove (GTK_LIST_STORE (capplet->priv->model),
				       &iter);
		return;

	} else {
		g_warning ("Unknown setting '%s'!", s + 1);
		g_free (key);
		return;
	}

	/*
	 * Figure out the number of the camera. Stupid GConf. Check that the
	 * directory still exists.
	 */
	key[s - key] = '\0';
	if (!gconf_client_dir_exists (capplet->priv->client, key, NULL)) {
		g_free (key);
		return;
	}
	if (!strrchr (key, '/')) {
		g_warning ("Invalid setting '%s'!", key);
		g_free (key);
		return;
	}
	n = atoi (strrchr (key, '/') + 1);
	g_free (key);
	if (!gnocam_capplet_get_iter (capplet, n, &iter)) {
		g_warning ("Could not find camera %i in list!");
		return;
	}

	value = gconf_entry_get_value (entry);
	gtk_list_store_set (GTK_LIST_STORE (capplet->priv->model), &iter,
			    c, gconf_value_get_string (value), -1);
}

static void
on_name_edited (GtkCellRendererText *cell, const gchar *path,
		const gchar *new_text, GnoCamCapplet *capplet)
{
	GtkTreeIter iter;
	guint n;
	GValue v = {0, };
	gchar *k, *s;

	g_return_if_fail (GNOCAM_IS_CAPPLET (capplet));

	/* Last entry edited? Add a new empty line. */
	gtk_tree_model_get_iter_from_string (capplet->priv->model, &iter, path);
	if (!memcmp (&iter, &capplet->priv->last, sizeof (GtkTreeIter)))
		gnocam_capplet_add_entry (capplet);

	/* Pass the new setting to gconf */
	gtk_tree_model_get_value (capplet->priv->model, &iter, COL_NUMBER, &v);
	n = g_value_get_uint (&v);
	g_value_unset (&v);
	k = g_strdup_printf ("/desktop/gnome/cameras/%i/name", n);
	s = gconf_client_get_string (capplet->priv->client, k, NULL);
	if (s && !strcmp (s, new_text)) {
		g_free (s);
		g_free (k);
		return;
	}       
	g_free (s);
	gconf_client_set_string (capplet->priv->client, k, new_text, NULL);
	g_free (k);
}

static void
on_model_edited (GtkCellRendererText *cell, const gchar *path,
		 const gchar *new_text, GnoCamCapplet *capplet)
{
	GtkTreeIter iter;
	GValue v = {0,};
	guint n;
	gchar *k, *s;

	/* Pass the new setting to gconf */
	gtk_tree_model_get_iter_from_string (capplet->priv->model, &iter, path);
	gtk_tree_model_get_value (capplet->priv->model, &iter, COL_NUMBER, &v);
	n = g_value_get_uint (&v);
	g_value_unset (&v);
	k = g_strdup_printf ("/desktop/gnome/cameras/%i/model", n);
	s = gconf_client_get_string (capplet->priv->client, k, NULL);
	if (s && !strcmp (s, new_text)) {
		g_free (s);
		g_free (k);
		return;
	}       
	g_free (s);
	gconf_client_set_string (capplet->priv->client, k, new_text, NULL);
	g_free (k);
}
		
static void
on_port_edited (GtkCellRendererText *cell, const gchar *path,
		const gchar *new_text, GnoCamCapplet *capplet)
{
	GtkTreeIter iter;
	GValue v = {0,};
	guint n;
	gchar *k, *s;

	/* Pass the new setting to gconf */
	gtk_tree_model_get_iter_from_string (capplet->priv->model, &iter, path);
	gtk_tree_model_get_value (capplet->priv->model, &iter, COL_NUMBER, &v);
	n = g_value_get_uint (&v);
	g_value_unset (&v); 
	k = g_strdup_printf ("/desktop/gnome/cameras/%i/port", n);
	s = gconf_client_get_string (capplet->priv->client, k, NULL);
	if (s && !strcmp (s, new_text)) {
		g_free (s);
		g_free (k);
		return;
	}
	g_free (s);
	gconf_client_set_string (capplet->priv->client, k, new_text, NULL);
	g_free (k);
}

static void
gnocam_capplet_remove_camera (GnoCamCapplet *capplet, guint n)
{
	gchar *k;

	g_return_if_fail (GNOCAM_IS_CAPPLET (capplet));

	k = g_strdup_printf ("/desktop/gnome/cameras/%i/name", n);
	gconf_client_unset (capplet->priv->client, k, NULL);
	g_free (k);
	k = g_strdup_printf ("/desktop/gnome/cameras/%i/model", n);
	gconf_client_unset (capplet->priv->client, k, NULL);
	g_free (k);
	k = g_strdup_printf ("/desktop/gnome/cameras/%i/port", n);
	gconf_client_unset (capplet->priv->client, k, NULL);
	g_free (k);
	k = g_strdup_printf ("/desktop/gnome/cameras/%i", n);
	gconf_client_unset (capplet->priv->client, k, NULL);
	g_free (k);
}

static void
delete_foreach_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
		     gpointer data)
{
	GnoCamCapplet *capplet = GNOCAM_CAPPLET (data);
	GValue v = {0, };
	guint n;
	gchar *k;

	gtk_tree_model_get_value (model, iter, COL_NUMBER, &v);
	n = g_value_get_uint (&v);
	g_value_unset (&v);
	gnocam_capplet_remove_camera (capplet, n);
}

static gboolean
on_tree_view_key_press_event (GtkWidget *widget, GdkEventKey *event,
			      GnoCamCapplet *capplet)
{
	GtkTreeSelection *s;

	switch (event->keyval) {
	case GDK_Delete:
		s = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
		gtk_tree_selection_selected_foreach (s, delete_foreach_func,
						     capplet);
		return TRUE;
	default:
		return FALSE;
	}
}

static gboolean
on_tree_view_button_press_event (GtkWidget *widget, GdkEventButton *event,
				 GnoCamCapplet *capplet)
{
	if (event->button == 3) {
		g_warning ("Implement popup!");
		return (TRUE);
	}

	return (FALSE);
}

static void
on_gtkam_toggled (GtkToggleButton *button, GnoCamCapplet *capplet)
{
	g_warning ("Not implemented!");
}

GtkWidget*
gnocam_capplet_new (GConfClient *client)
{
	GnoCamCapplet *capplet;
	GtkWidget *hbox, *image, *vbox, *w;
	GtkListStore *s;
	GtkCellRenderer *c;
	GtkTreeViewColumn *col;
	gchar *k = NULL, *name, *model, *port;
	guint n = 0;
	GtkTreeIter iter;

	capplet = gtk_type_new (GNOCAM_TYPE_CAPPLET);

	/* Watch out for changes */
	capplet->priv->client = client;
	g_object_ref (client);
	gconf_client_notify_add (client, "/desktop/gnome/cameras", notify_func,
				 capplet, NULL, NULL);

	/* Create a hbox */
	hbox = gtk_hbox_new (FALSE, 10);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (capplet)->vbox), hbox,
			    TRUE, TRUE, 10);

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
	g_signal_connect (w, "button_press_event",
		G_CALLBACK (on_tree_view_button_press_event), capplet);
	g_signal_connect (w, "key_press_event",
		G_CALLBACK (on_tree_view_key_press_event), capplet);

	/* Create the model for the table */
	s = gtk_list_store_new (NUM_COLS, G_TYPE_UINT, G_TYPE_STRING,
				G_TYPE_STRING,
				G_TYPE_STRING, G_TYPE_BOOLEAN);
	gtk_tree_view_set_model (GTK_TREE_VIEW (w), GTK_TREE_MODEL (s));
	capplet->priv->model = GTK_TREE_MODEL (s);

	/* Load the current settings. */
	while (n < 10) {
		k = g_strdup_printf ("/desktop/gnome/cameras/%i", n);
		if (gconf_client_dir_exists (client, k, NULL)) {
		    g_free (k);
		    k = g_strdup_printf ("/desktop/gnome/cameras/%i/name", n);
		    name = gconf_client_get_string (client, k, NULL);
		    g_free (k);
		    k = g_strdup_printf ("/desktop/gnome/cameras/%i/model", n);
		    model = gconf_client_get_string (client, k, NULL);
		    g_free (k);
		    k = g_strdup_printf ("/desktop/gnome/cameras/%i/port", n);
		    port = gconf_client_get_string (client, k, NULL);
		    g_free (k);
		    k = NULL;
		    gtk_list_store_append (s, &iter);
		    gtk_list_store_set (s, &iter, COL_IS_EDITABLE, TRUE,
			COL_NUMBER, n, -1);
		    if (name) {
			    gtk_list_store_set (s, &iter, COL_NAME, name, -1);
			    g_free (name);
		    }
		    if (model) {
			    gtk_list_store_set (s, &iter, COL_MODEL, model, -1);
			    g_free (model);
		    }
		    if (port) {
			    gtk_list_store_set (s, &iter, COL_PORT, port, -1);
			    g_free (port);
		    }
		}
		g_free (k);
		k = NULL;
		n++;
	}
	g_free (k);

	/* Create an empty line so that users can add their camera. */
	gnocam_capplet_add_entry (capplet);

	/* Add the column for the name */
	c = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Name"), c,
		"text", COL_NAME, "editable", COL_IS_EDITABLE, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), col);
	g_signal_connect (c, "edited", G_CALLBACK (on_name_edited), capplet);

	/* Add the column for the model */
	c = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Model"), c,
		"text", COL_MODEL, "editable", COL_IS_EDITABLE, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), col);
	g_signal_connect (c, "edited", G_CALLBACK (on_model_edited), capplet);

	/* Add the column for the port */
	c = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Port"), c,
		"text", COL_PORT, "editable", COL_IS_EDITABLE, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), col);
	g_signal_connect (c, "edited", G_CALLBACK (on_port_edited), capplet);

	/* Add the check button for gtkam's settings. */
	w = gtk_check_button_new_with_label (_("Write settings for gtkam"));
	gtk_widget_show (w);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (capplet)->vbox), w,
			    FALSE, FALSE, 0);
	g_signal_connect (w, "toggled", G_CALLBACK (on_gtkam_toggled), capplet);

	/* Add the ok-button. */
	w = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_widget_show (w);
	gtk_box_pack_end (GTK_BOX (GTK_DIALOG (capplet)->action_area), w,
			  FALSE, FALSE, 0);
	g_signal_connect (w, "clicked", G_CALLBACK (on_ok_clicked), capplet);

	return (GTK_WIDGET (capplet));
}
