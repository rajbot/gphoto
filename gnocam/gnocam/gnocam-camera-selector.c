#include <config.h>
#include "gnocam-camera-selector.h"

#include <gal/util/e-util.h>
#include <gtk/gtkclist.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkhbox.h>
#include <libgnomeui/gnome-stock.h>
#include <libgnomeui/gnome-dialog-util.h>
#include <libgnome/gnome-i18n.h>

#define PARENT_TYPE GNOME_TYPE_DIALOG
static GnomeDialogClass *parent_class;

struct _GnoCamCameraSelectorPrivate
{
	GConfClient *client;

	GSList *list;

	GtkWidget *clist;
};

static void
gnocam_camera_selector_destroy (GtkObject *object)
{
	GnoCamCameraSelector *selector;

	selector = GNOCAM_CAMERA_SELECTOR (object);

	gtk_object_unref (GTK_OBJECT (selector->priv->client));

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_camera_selector_finalize (GtkObject *object)
{
	GnoCamCameraSelector *selector;

	selector = GNOCAM_CAMERA_SELECTOR (object);

	g_free (selector->priv);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_camera_selector_class_init (GnoCamCameraSelectorClass *klass)
{
	GtkObjectClass *object_class = GTK_OBJECT_CLASS (klass);

	parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = gnocam_camera_selector_destroy;
	object_class->finalize = gnocam_camera_selector_finalize;
}

static void
gnocam_camera_selector_init (GnoCamCameraSelector *selector)
{
	selector->priv = g_new0 (GnoCamCameraSelectorPrivate, 1);
}

E_MAKE_TYPE (gnocam_camera_selector, "GnoCamCameraSelector", GnoCamCameraSelector, gnocam_camera_selector_class_init, gnocam_camera_selector_init, PARENT_TYPE)

static void
on_check_toggled (GtkToggleButton *toggle, gpointer data)
{
	GnoCamCameraSelector *selector;

	selector = GNOCAM_CAMERA_SELECTOR (data);

	gnome_error_dialog ("Not implemented!");
}

GnomeDialog *
gnocam_camera_selector_new (GConfClient *client)
{
	GnoCamCameraSelector *selector;
	const gchar *buttons[] = {GNOME_STOCK_BUTTON_OK,
				  GNOME_STOCK_BUTTON_CANCEL, NULL};
	const gchar *text[] = {NULL, NULL};
	gint i;
	GtkWidget *hbox, *pixmap, *vbox, *check;
	
	g_return_val_if_fail (client, NULL);

	selector = gtk_type_new (GNOCAM_TYPE_CAMERA_SELECTOR);
	gnome_dialog_constructv (GNOME_DIALOG (selector), _("Select a Camera"),
				 buttons);
	gnome_dialog_grab_focus (GNOME_DIALOG (selector), 0);

	selector->priv->client = client;
	gtk_object_ref (GTK_OBJECT (client));
	selector->priv->list = gconf_client_get_list (client,
			"/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, NULL);

	hbox = gtk_hbox_new (FALSE, 10);
	gtk_widget_show (hbox);
	gtk_container_add (GTK_CONTAINER (GNOME_DIALOG (selector)->vbox), hbox);
	
	vbox = gtk_vbox_new (FALSE, 10);
	gtk_widget_show (vbox);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
	pixmap = gnome_pixmap_new_from_file (IMAGEDIR "/gnocam.png");
	gtk_widget_show (pixmap);
	gtk_box_pack_start (GTK_BOX (vbox), pixmap, FALSE, FALSE, 0);
	check = gtk_check_button_new_with_label (_("Do not ask again"));
	gtk_widget_show (check);
	gtk_box_pack_start (GTK_BOX (vbox), check, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (check), "toggled", 
			    GTK_SIGNAL_FUNC (on_check_toggled), selector);

	selector->priv->clist = gtk_clist_new (1);
	gtk_widget_show (selector->priv->clist);
	gtk_box_pack_start (GTK_BOX (hbox), selector->priv->clist, TRUE, 
			    TRUE, 0);
	
	for (i = 0; i < g_slist_length (selector->priv->list); i += 3) {
		text[0] = g_slist_nth_data (selector->priv->list, i);
		gtk_clist_append (GTK_CLIST (selector->priv->clist),
				  (gchar **) text);
	}

	if (g_slist_length (selector->priv->list))
		gtk_clist_select_row (GTK_CLIST (selector->priv->clist), 0, 0);

	return (GNOME_DIALOG (selector));
}

const gchar *
gnocam_camera_selector_get_name (GnoCamCameraSelector *selector)
{
	GList *selection;
	const gchar *name;

	selection = GTK_CLIST (selector->priv->clist)->selection;
	if (!g_list_length (selection))
		return (NULL);

	if (g_list_length (selection) > 1)
		g_warning ("More than one entry selected!");

	name = g_slist_nth_data (selector->priv->list, 
				 GPOINTER_TO_INT (selection->data) * 3);
	return (name);
}
