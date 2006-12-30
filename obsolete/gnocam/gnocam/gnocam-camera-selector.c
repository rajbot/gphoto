#include "config.h"
#include "gnocam-camera-selector.h"

#include <gal/util/e-util.h>
#include <gtk/gtksignal.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkhbox.h>
#include <libgnomeui/gnome-stock.h>
#include <libgnomeui/gnome-dialog-util.h>
#include <libgnome/gnome-i18n.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-object.h>
#include <bonobo-conf/bonobo-property-editor.h>

#include <gnocam-capplet-table-scrolled.h>

#define PARENT_TYPE GNOME_TYPE_DIALOG
static GnomeDialogClass *parent_class;

struct _GnoCamCameraSelectorPrivate
{
	const gchar *name;

	GtkWidget *table;
};

static void
gnocam_camera_selector_destroy (GtkObject *object)
{
	GnoCamCameraSelector *selector;

	selector = GNOCAM_CAMERA_SELECTOR (object);

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

GnomeDialog *
gnocam_camera_selector_new (void)
{
	GnoCamCameraSelector *selector;
	const gchar *buttons[] = {GNOME_STOCK_BUTTON_OK,
				  GNOME_STOCK_BUTTON_CANCEL, NULL};
	GtkWidget *hbox, *pixmap, *vbox, *widget, *label;
	Bonobo_PropertyBag pb;
	CORBA_Environment ev;
	
	selector = gtk_type_new (GNOCAM_TYPE_CAMERA_SELECTOR);
	gnome_dialog_constructv (GNOME_DIALOG (selector), _("Select a Camera"),
				 buttons);
	gnome_dialog_set_default (GNOME_DIALOG (selector), 0);
	gtk_window_set_policy (GTK_WINDOW (selector), TRUE, TRUE, TRUE);
	gtk_widget_set_usize (GTK_WIDGET (selector), 400, 300);

	hbox = gtk_hbox_new (FALSE, 10);
	gtk_widget_show (hbox);
	gtk_container_add (GTK_CONTAINER (GNOME_DIALOG (selector)->vbox), hbox);
	
	vbox = gtk_vbox_new (FALSE, 10);
	gtk_widget_show (vbox);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

	/* The camera picture */
	pixmap = gnome_pixmap_new_from_file (IMAGEDIR "/gnocam.png");
	gtk_widget_show (pixmap);
	gtk_box_pack_start (GTK_BOX (vbox), pixmap, FALSE, FALSE, 0);

	/* The "Autodetect" check button */
	CORBA_exception_init (&ev);
	pb = bonobo_get_object ("config:/apps/" PACKAGE,
				"Bonobo/PropertyBag", &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not get property bag: %s", 
			   bonobo_exception_get_text (&ev));
	} else {
		GtkObject *ed;

		ed = bonobo_peditor_new (pb, "autodetect", TC_boolean, NULL);
		bonobo_object_release_unref (pb, NULL);
		widget = bonobo_peditor_get_widget (BONOBO_PEDITOR (ed));
		gtk_widget_show (widget);
		gtk_box_pack_end (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
		label = gtk_label_new (_("Autodetect"));
		gtk_widget_show (label);
		gtk_container_add (GTK_CONTAINER (widget), label);
	}
	CORBA_exception_free (&ev);

	/* The camera list */
	selector->priv->table = gnocam_capplet_table_scrolled_new (NULL);
	gtk_widget_show (selector->priv->table);
	gtk_box_pack_start (GTK_BOX (hbox), selector->priv->table,
			    TRUE, TRUE, 0);
	
	return (GNOME_DIALOG (selector));
}

static void
foreach_name (gint row, gpointer data)
{
	GnoCamCameraSelector *selector;
	ETable *table;

	selector = GNOCAM_CAMERA_SELECTOR (data);
	table = GNOCAM_CAPPLET_TABLE_SCROLLED (selector->priv->table)->table;

	selector->priv->name = e_table_model_value_at (table->model, 0, row);
}

const gchar *
gnocam_camera_selector_get_name (GnoCamCameraSelector *selector)
{
	ETable *table;

	table = GNOCAM_CAPPLET_TABLE_SCROLLED (selector->priv->table)->table;

	if (e_table_selected_count (table) != 1)
		g_warning ("Not exactly one entry selected!");

	e_table_selected_row_foreach (table, foreach_name, selector);

	return (selector->priv->name);
}
