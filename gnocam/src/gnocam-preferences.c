#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnocam-preferences.h>

#include <gal/util/e-util.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#define PARENT_TYPE GNOME_TYPE_DIALOG
static GnomeDialogClass* parent_class = NULL;

struct _GnoCamPreferencesPrivate {
	GConfClient 	*client;
};


/*************/
/* Callbacks */
/*************/

static void
on_control_center_clicked (GtkButton* button, gpointer user_data)
{
	gchar* argv [2] = {"camera-capplet", NULL};

	if (gnome_execute_async (NULL, 1, argv) < 0) g_warning (_("Cannot execute camera-capplet!"));
}

static void
on_prefix_changed (GtkEditable *editable, GnoCamPreferences *preferences)
{
	gconf_client_set_string (preferences->priv->client, "/apps/" PACKAGE "/prefix", gtk_entry_get_text (GTK_ENTRY (editable)), NULL);
}

/*************************/
/* Gnome-Dialog specific */
/*************************/

static void
gnocam_preferences_destroy (GtkObject* object)
{
	GnoCamPreferences*	preferences;

	preferences = GNOCAM_PREFERENCES (object);

	gtk_object_unref (GTK_OBJECT (preferences->priv->client));

	g_free (preferences->priv);
	preferences->priv = NULL;

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_preferences_class_init (GnoCamPreferencesClass* klass)
{
	GtkObjectClass* 	object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = gnocam_preferences_destroy;

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_preferences_init (GnoCamPreferences* preferences)
{
	preferences->priv = g_new (GnoCamPreferencesPrivate, 1);
}

GtkWidget*
gnocam_preferences_new (GtkWindow* window, GConfClient *client)
{
	GdkPixbuf*		pixbuf;
	GdkPixmap*		pixmap = NULL;
	GdkBitmap*		bitmap = NULL;
	GnoCamPreferences*	new;
	GtkWidget*		label;
	GtkWidget*		hbox;
	GtkWidget*		vbox;
	GtkWidget*		widget;
	GtkWidget*		frame;
	GtkWidget*		notebook;
	GtkWidget*		button;
	const gchar*		buttons [] = {GNOME_STOCK_BUTTON_OK, NULL};
	gchar*			tmp;

	new = gtk_type_new (GNOCAM_TYPE_PREFERENCES);

	new->priv->client = client;
	gtk_object_ref (GTK_OBJECT (client));

	gnome_dialog_constructv (GNOME_DIALOG (new), _(PACKAGE " - Preferences"), buttons);
	gnome_dialog_set_close (GNOME_DIALOG (new), TRUE);
	gnome_dialog_set_parent (GNOME_DIALOG (new), window);

	/* Create the notebook */
	gtk_widget_show (notebook = gtk_notebook_new ());
	gtk_container_add (GTK_CONTAINER (GNOME_DIALOG (new)->vbox), notebook);

        /* Create the page for GnoCam settings */
        gtk_widget_show (vbox = gtk_vbox_new (FALSE, 0));
        gtk_widget_show (label = gtk_label_new (PACKAGE));
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
        gtk_widget_show (frame = gtk_frame_new (_("Default Prefix")));
        gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
        gtk_container_add (GTK_CONTAINER (vbox), frame);
        gtk_widget_show (vbox = gtk_vbox_new (FALSE, 0));
        gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);
        gtk_container_add (GTK_CONTAINER (frame), vbox);
	gtk_widget_show (widget = gtk_entry_new ());
        gtk_container_add (GTK_CONTAINER (vbox), widget);
	tmp = gconf_client_get_string (client, "/apps/" PACKAGE "/prefix", NULL);
	gtk_entry_set_text (GTK_ENTRY (widget), tmp);
	g_free (tmp);
	gtk_signal_connect (GTK_OBJECT (widget), "changed", GTK_SIGNAL_FUNC (on_prefix_changed), new);

	/* Create the page for GNOME settings */
	gtk_widget_show (hbox = gtk_hbox_new (FALSE, 10));
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
	gtk_widget_show (label = gtk_label_new (_("GNOME")));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, label);
	gtk_widget_show (label = gtk_label_new (_("For GNOME settings, please use the control-center. Thank you!")));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_container_add (GTK_CONTAINER (hbox), label);
	gtk_widget_show (button = gtk_button_new ());
	gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (on_control_center_clicked), new);
	gtk_container_add (GTK_CONTAINER (hbox), button);
	gtk_widget_show (hbox = gtk_hbox_new (FALSE, 5));
	gtk_container_add (GTK_CONTAINER (button), hbox);
	pixbuf = gdk_pixbuf_new_from_file (IMAGEDIR "/control-center.png");
	gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap, 4);
	gtk_widget_show (widget = gtk_pixmap_new (pixmap, bitmap));
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 10);
	gtk_widget_show (label = gtk_label_new (_("Control-Center")));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 10);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_preferences, "GnoCamPreferences", GnoCamPreferences, gnocam_preferences_class_init, gnocam_preferences_init, PARENT_TYPE)

