#include <config.h>
#include "gnocam-chooser.h"

#include "GNOME_C.h"

#include <string.h>

#include <gtk/gtkbox.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkcontainer.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtktreeviewcolumn.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtktogglebutton.h>

#include <glade/glade.h>

#include <bonobo-activation/bonobo-activation-activate.h>

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-object.h>

#define _(s) s

struct _GnocamChooserPriv
{
	GList *l;
	GtkToggleButton *toggle_connect_auto;

	GtkListStore *manufs, *models, *ports;
	GtkTreeSelection *s_manufs, *s_models, *s_ports;
};

#define PARENT_TYPE GTK_TYPE_DIALOG
static GtkDialogClass *parent_class;

enum {
	NAME_COL = 0,
	NUM_COLS
};

enum {
	CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

static void
gnocam_chooser_finalize (GObject *object)
{
	GnocamChooser *c = GNOCAM_CHOOSER (object);
	guint i;

	for (i = 0; i < g_list_length (c->priv->l); i++)
		CORBA_free (g_list_nth_data (c->priv->l, i));
	g_list_free (c->priv->l);
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

	signals[CHANGED] = g_signal_new ("changed",
		G_TYPE_FROM_CLASS (g_class), G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET (GnocamChooserClass, changed), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static void
gnocam_chooser_init (GTypeInstance *instance, gpointer g_class)
{
	GnocamChooser *c = GNOCAM_CHOOSER (instance);

	c->priv = g_new0 (GnocamChooserPriv, 1);

	c->priv->manufs = gtk_list_store_new (NUM_COLS, G_TYPE_STRING);
	c->priv->models = gtk_list_store_new (NUM_COLS, G_TYPE_STRING);
	c->priv->ports = gtk_list_store_new (NUM_COLS, G_TYPE_STRING);
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
	guint i, j, k;
	GNOME_C_Mngr_ManufacturerList *ml;
	GtkTreeIter iter;
	GList *list = NULL;

	/* Clear the internal list */
	for (i = 0; i < g_list_length (c->priv->l); i++)
		CORBA_free (g_list_nth_data (c->priv->l, i));
	g_list_free (c->priv->l);
	c->priv->l = NULL;

	/* Clear the lists */
	gtk_list_store_clear (c->priv->manufs);
	gtk_list_store_clear (c->priv->models);
	gtk_list_store_clear (c->priv->ports);

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
		if (BONOBO_EX (&ev))
			continue;
		ml = GNOME_C_Mngr_get_devices (m, &ev);
		bonobo_object_release_unref (m, NULL);
		if (BONOBO_EX (&ev))
			continue;
		c->priv->l = g_list_append (c->priv->l, ml);
	}
	CORBA_free (l);
	CORBA_exception_free (&ev);

	for (i = 0; i < g_list_length (c->priv->l); i++) {
	    ml = g_list_nth_data (c->priv->l, i);
	    for (j = 0; j < ml->_length; j++) {
		for (k = 0; k < g_list_length (list); k++)
		    if (!strcmp (g_list_nth_data (list, k),
				 ml->_buffer[j].manufacturer)) break;
		if (k == g_list_length (list))
		    list = g_list_append (list, ml->_buffer[j].manufacturer);
	    }
	}
	for (i = 0; i < g_list_length (list); i++) {
		gtk_list_store_append (c->priv->manufs, &iter);
		gtk_list_store_set (c->priv->manufs, &iter, NAME_COL,
				    g_list_nth_data (list, i), -1);
	}
	g_list_free (list);
}

static void
on_close_clicked (GtkButton *button, GnocamChooser *c)
{
	gtk_object_destroy (GTK_OBJECT (c));
}

static void
on_manuf_selection_changed (GtkTreeSelection *s, GnocamChooser *c)
{
	gchar *manuf;
	guint i, j, k, n;
	GList *list = NULL;
	GNOME_C_Mngr_ManufacturerList *ml;
	GNOME_C_Mngr_ModelList l;
	GtkTreeIter iter;

	manuf = gnocam_chooser_get_manufacturer (c);
	g_return_if_fail (manuf != NULL);

	/* Set up the list for the models */
	gtk_list_store_clear (c->priv->models);
	for (i = 0; i < g_list_length (c->priv->l); i++) {
	    ml = g_list_nth_data (c->priv->l, i);
	    for (j = 0; j < ml->_length; j++) {
		if (strcmp (ml->_buffer[j].manufacturer, manuf)) continue;
		l = ml->_buffer[j].models;
		for (k = 0; k < l._length; k++) {
		    for (n = 0; n < g_list_length (list); n++)
			if (!strcmp (g_list_nth_data (list, n),
				     l._buffer[k].model)) break;
		    if (n == g_list_length (list))
			list = g_list_append (list, l._buffer[k].model);
		}
	    }
	}
	g_free (manuf);
	for (i = 0; i < g_list_length (list); i++) {
		gtk_list_store_append (c->priv->models, &iter);
		gtk_list_store_set (c->priv->models, &iter, NAME_COL,
				g_list_nth_data (list, i), -1);
	}
	g_list_free (list);

	/* Select the first model */
	gnocam_chooser_set_model (c, NULL);

	g_signal_emit (c, signals[CHANGED], 0);
}

static void
on_port_selection_changed (GtkTreeSelection *s, GnocamChooser *c)
{
	g_signal_emit (c, signals[CHANGED], 0);
}

static void
on_model_selection_changed (GtkTreeSelection *s, GnocamChooser *c)
{
	GtkTreeIter iter;
	guint i, j, k, l, n;
	GList *list = NULL;
	GNOME_C_Mngr_ManufacturerList *ml;
	GNOME_C_Mngr_PortList pl;
	gchar *manuf, *model, *port;

	model = gnocam_chooser_get_model (c); if (!model) return;
	manuf = gnocam_chooser_get_manufacturer (c);
	g_return_if_fail (manuf != NULL);
	port  = gnocam_chooser_get_port (c);

	/* Set up the list for the ports */
	gtk_list_store_clear (c->priv->ports);
	for (i = 0; i < g_list_length (c->priv->l); i++) {
	    ml = g_list_nth_data (c->priv->l, i);
	    for (j = 0; j < ml->_length; j++) {
		if (strcmp (ml->_buffer[j].manufacturer, manuf)) continue;
		for (k = 0; k < ml->_buffer[j].models._length; k++) {
		    if (strcmp (ml->_buffer[j].models._buffer[k].model,
				model)) continue;
		    pl = ml->_buffer[j].models._buffer[k].ports;
		    for (l = 0; l < pl._length; l++) {
		        for (n = 0; n < g_list_length (list); n++)
			    if (!strcmp (g_list_nth_data (list, n),
					 pl._buffer[l])) break;
			if (n == g_list_length (list))
			    list = g_list_append (list, pl._buffer[l]);
		    }
		}
	    }
	}
	g_free (model);
	g_free (manuf);
	for (i = 0; i < g_list_length (list); i++) {
		gtk_list_store_append (c->priv->ports, &iter);
		gtk_list_store_set (c->priv->ports, &iter, NAME_COL,
				g_list_nth_data (list, i), -1);
	}
	g_list_free (list);

	gnocam_chooser_set_port (c, port);
	g_free (port);

	g_signal_emit (c, signals[CHANGED], 0);
}

static void
gnocam_chooser_set_common (GnocamChooser *c, GtkTreeModel *m,
			   GtkTreeSelection *s, const gchar *n)
{
	GtkTreeIter iter;
	GValue v = {0,};

	g_return_if_fail (GNOCAM_IS_CHOOSER (c));
	g_return_if_fail (GTK_IS_TREE_MODEL (m));
	g_return_if_fail (GTK_IS_TREE_SELECTION (s));

	if (!n) {
		if (gtk_tree_model_get_iter_first (m, &iter))
			gtk_tree_selection_select_iter (s, &iter);
		return;
	}

	if (gtk_tree_model_get_iter_first (m, &iter)) {
	    gtk_tree_model_get_value (m, &iter, NAME_COL, &v);
	    if (!strcmp (g_value_get_string (&v), n)) {
		g_value_unset (&v);
		gtk_tree_selection_select_iter (s, &iter);
	        return;
	    }
	    g_value_unset (&v);
	    while (gtk_tree_model_iter_next (m, &iter)) {
		gtk_tree_model_get_value (m, &iter, NAME_COL, &v);
		if (!strcmp (g_value_get_string (&v), n)) {
		    g_value_unset (&v);
		    gtk_tree_selection_select_iter (s, &iter);
		    return;
		}
		g_value_unset (&v);
	    }
	}
}

void
gnocam_chooser_set_port (GnocamChooser *c, const gchar *port)
{
	g_return_if_fail (GNOCAM_IS_CHOOSER (c));

	gnocam_chooser_set_common (c, GTK_TREE_MODEL (c->priv->ports),
				   c->priv->s_ports, port);
}

void
gnocam_chooser_set_model (GnocamChooser *c, const gchar *model)
{
	g_return_if_fail (GNOCAM_IS_CHOOSER (c));

	gnocam_chooser_set_common (c, GTK_TREE_MODEL (c->priv->models),
				   c->priv->s_models, model);
}

void
gnocam_chooser_set_manufacturer (GnocamChooser *c, const gchar *manufacturer)
{
	g_return_if_fail (GNOCAM_IS_CHOOSER (c));

	gnocam_chooser_set_common (c, GTK_TREE_MODEL (c->priv->manufs),
				   c->priv->s_manufs, manufacturer);
}

static gchar *
gnocam_chooser_get_common (GnocamChooser *c, GtkTreeSelection *s)
{
	GtkTreeIter iter;
	GtkTreeModel *m;
	GValue v = {0,};
	gchar *n;

	g_return_val_if_fail (GNOCAM_IS_CHOOSER (c), NULL);
	g_return_val_if_fail (GTK_IS_TREE_SELECTION (s), NULL);

	if (!gtk_tree_selection_get_selected (s, &m, &iter)) return NULL;
	gtk_tree_model_get_value (m, &iter, NAME_COL, &v);
	g_return_val_if_fail (G_VALUE_HOLDS_STRING (&v), NULL);
	n = g_strdup (g_value_get_string (&v));
	g_value_unset (&v);

	return n;
}

gchar *
gnocam_chooser_get_manufacturer (GnocamChooser *c)
{
	g_return_val_if_fail (GNOCAM_IS_CHOOSER (c), NULL);

	return gnocam_chooser_get_common (c, c->priv->s_manufs);
}

gchar *
gnocam_chooser_get_model (GnocamChooser *c)
{
	g_return_val_if_fail (GNOCAM_IS_CHOOSER (c), NULL);

	return gnocam_chooser_get_common (c, c->priv->s_models);
}

gchar *
gnocam_chooser_get_port (GnocamChooser *c)
{
	g_return_val_if_fail (GNOCAM_IS_CHOOSER (c), NULL);

	return gnocam_chooser_get_common (c, c->priv->s_ports);
}

static void
on_connect_auto_toggled (GtkToggleButton *toggle, GnocamChooser *c)
{
	g_signal_emit (c, signals[CHANGED], 0);
}

GnocamChooser *
gnocam_chooser_new (void)
{
	GnocamChooser *c = g_object_new (GNOCAM_TYPE_CHOOSER, NULL);
	GladeXML *xml;
	GtkWidget *ui, *b, *w;
	GtkCellRenderer *r;
	GtkTreeViewColumn *col;
	GtkTreeIter iter;

	/* Read the interface description */
	xml = glade_xml_new (GNOCAM_GLADE_DIR "/gnocam.glade",
			     "hbox_contents", NULL);
	if (!xml) xml = glade_xml_new (GNOCAM_SRCDIR "/libgnocam/gnocam.glade",
				       "hbox_contents", NULL);
	if (!xml) return NULL;
	ui = glade_xml_get_widget (xml, "hbox_contents");
	if (!ui) {
		g_object_unref (xml);
		return NULL;
	}

	/* Create the contents */
	gtk_widget_show (ui);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (c)->vbox), ui, TRUE, TRUE, 0);

	/* Add the buttons */
	b = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_widget_show (b);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (c)->action_area), b,
			    TRUE, TRUE, 0);
	g_signal_connect (b, "clicked", G_CALLBACK (on_close_clicked), c);

	/* Manufacturer */
	w = glade_xml_get_widget (xml, "treeview_manufacturer");
	gtk_tree_view_set_model (GTK_TREE_VIEW (w),
				 GTK_TREE_MODEL (c->priv->manufs));
	r = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Manufacturer"), r,
		"text", NAME_COL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), col);
	c->priv->s_manufs = gtk_tree_view_get_selection (GTK_TREE_VIEW (w));
	g_signal_connect (c->priv->s_manufs, "changed",
			  G_CALLBACK (on_manuf_selection_changed), c);

	/* Model */
	w = glade_xml_get_widget (xml, "treeview_model");
	gtk_tree_view_set_model (GTK_TREE_VIEW (w),
				 GTK_TREE_MODEL (c->priv->models));
	r = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Model"), r,
		"text", NAME_COL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), col);
	c->priv->s_models = gtk_tree_view_get_selection (GTK_TREE_VIEW (w));
	g_signal_connect (c->priv->s_models, "changed",
			  G_CALLBACK (on_model_selection_changed), c);

	/* Port */
	w = glade_xml_get_widget (xml, "treeview_port");
	gtk_tree_view_set_model (GTK_TREE_VIEW (w),
				 GTK_TREE_MODEL (c->priv->ports));
	r = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Port"), r,
		"text", NAME_COL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), col);
	c->priv->s_ports = gtk_tree_view_get_selection (GTK_TREE_VIEW (w));
	g_signal_connect (c->priv->s_ports, "changed",
			  G_CALLBACK (on_port_selection_changed), c);

	/* Connect automatically */
	w = glade_xml_get_widget (xml, "checkbutton_connect_auto");
	c->priv->toggle_connect_auto = GTK_TOGGLE_BUTTON (w);
	g_signal_connect (w, "toggled",
			  G_CALLBACK (on_connect_auto_toggled), c);

	/* Find available cameras on the system. */
	gnocam_chooser_setup (c);

	/* Select the first manufacturer */
	if (gtk_tree_model_get_iter_first (
			GTK_TREE_MODEL (c->priv->manufs), &iter))
		gtk_tree_selection_select_iter (c->priv->s_manufs, &iter);

	/* We don't need the GLADE xml any more. */
	g_object_unref (xml);

	return c;
}

void
gnocam_chooser_set_connect_auto (GnocamChooser *c, gboolean connect_auto)
{
	g_return_if_fail (GNOCAM_IS_CHOOSER (c));
	gtk_toggle_button_set_active (c->priv->toggle_connect_auto,
				      connect_auto);
}

gboolean
gnocam_chooser_get_connect_auto (GnocamChooser *c)
{
	g_return_val_if_fail (GNOCAM_IS_CHOOSER (c), FALSE);
	return gtk_toggle_button_get_active (c->priv->toggle_connect_auto);
}
