#include "gnocam-capture.h"

#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-stock.h>
#include <gal/util/e-util.h>

#include "GnoCam.h"

#define PARENT_TYPE GNOME_TYPE_DIALOG
static GnomeDialogClass* parent_class = NULL;

struct _GnoCamCapturePrivate {
	Camera    *camera;

	GtkWidget *frame;
	GtkWidget *preview;
	GtkWidget *spin;
	GtkObject *adjustment;

	guint do_capture_timeout;
};

static gboolean 
do_capture (gpointer data){
	GnoCamCapture *capture;
	CameraFile *file;
	gint result;
	GdkPixbufLoader *loader;
	GdkPixbuf *pixbuf;
	GdkPixmap *pixmap;
	GdkBitmap *bitmap;

	capture = GNOCAM_CAPTURE (data);

        /* Prepare the image. */
	gp_file_new (&file);

        /* Capture. */
	result = gp_camera_capture_preview (capture->priv->camera, file);
	if (result != GP_OK) {
		g_warning (_("Could not capture!\n(%s)"),
			   gp_camera_get_result_as_string (
				   	capture->priv->camera, result));
		gp_file_unref (file);
		return (TRUE);
	}

	loader = gdk_pixbuf_loader_new ();
	gdk_pixbuf_loader_write (loader, file->data, file->size);
	gp_file_unref (file);
	gdk_pixbuf_loader_close (loader);

	pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
	gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap, 1);
	gtk_object_unref (GTK_OBJECT (loader));

	/* Destroy old widget */
	if (capture->priv->preview)
		gtk_container_remove (GTK_CONTAINER (capture->priv->frame),
				      capture->priv->preview);

	/* Show new widget */
	capture->priv->preview = gtk_pixmap_new (pixmap, bitmap);
	gtk_widget_show (capture->priv->preview);
	gtk_container_add (GTK_CONTAINER (capture->priv->frame),
			   capture->priv->preview);

	return (TRUE);
}

static void
gnocam_capture_destroy (GtkObject* object)
{
        GnoCamCapture*  capture;

        capture = GNOCAM_CAPTURE (object);

	if (capture->priv->do_capture_timeout)
		gtk_timeout_remove (capture->priv->do_capture_timeout);

	if (capture->priv->camera) {
        	gp_camera_unref (capture->priv->camera);
		capture->priv->camera = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_capture_finalize (GtkObject* object)
{
	GnoCamCapture*  capture;

	capture = GNOCAM_CAPTURE (object);

	g_free (capture->priv);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_capture_class_init (GnoCamCaptureClass* klass)
{
	GtkObjectClass*	object_class;

	object_class = (GtkObjectClass*) klass;
	object_class->destroy  = gnocam_capture_destroy;
	object_class->finalize = gnocam_capture_finalize;

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_capture_init (GnoCamCapture* capture)
{
	capture->priv = g_new0 (GnoCamCapturePrivate, 1);
}

static void
on_adjustment_value_changed (GtkAdjustment *adjustment, gpointer data)
{
	GnoCamCapture *capture;

	capture = GNOCAM_CAPTURE (data);

	if (capture->priv->do_capture_timeout)
		gtk_timeout_remove (capture->priv->do_capture_timeout);
	capture->priv->do_capture_timeout =
		gtk_timeout_add (adjustment->value * 1000, do_capture, capture);
}

static void
on_toggle_toggled (GtkToggleButton *toggle, gpointer data)
{
	GnoCamCapture *capture;

	capture = GNOCAM_CAPTURE (data);
	
	gtk_widget_set_sensitive (capture->priv->spin, toggle->active);
	if (toggle->active) {
		if (!capture->priv->do_capture_timeout) {
			capture->priv->do_capture_timeout =
				gtk_timeout_add (GTK_ADJUSTMENT (capture->
					priv->adjustment)->value * 1000,
						do_capture, capture);
		}
	} else {
		if (capture->priv->do_capture_timeout) {
			gtk_timeout_remove (capture->priv->do_capture_timeout);
			capture->priv->do_capture_timeout = 0;
		}
	}
}

GnomeDialog*
gnocam_capture_new (Camera *camera, CORBA_Environment *ev)
{
	GtkWidget *hbox, *check, *label, *pixmap;
	GnoCamCapture *new;
	const gchar *buttons[] = {_("Capture"), GNOME_STOCK_BUTTON_CANCEL,
				  NULL};
	
	gtk_widget_push_colormap (gdk_rgb_get_cmap ());
	gtk_widget_push_visual (gdk_rgb_get_visual ());

	bonobo_return_val_if_fail (camera, NULL, ev);

	/* Does the camera support capturing images? */
	if (!(camera->abilities->operations & GP_OPERATION_CAPTURE_IMAGE)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_GNOME_Camera_NotSupported, NULL);
		return (NULL);
	}

	new = gtk_type_new (GNOCAM_TYPE_CAPTURE);
	gnome_dialog_constructv (GNOME_DIALOG (new), _("Capture Image"),
				 buttons);
	gnome_dialog_set_default (GNOME_DIALOG (new), 0);
	gnome_dialog_set_close (GNOME_DIALOG (new), TRUE);
	
	new->priv->camera = camera;
	gp_camera_ref (camera);

	/* First hbox */
	hbox = gtk_hbox_new (FALSE, 10);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (new)->vbox), hbox,
			    TRUE, TRUE, 0);
	pixmap = gnome_pixmap_new_from_file (IMAGEDIR "/gnocam.png");
	gtk_widget_show (pixmap);
	gtk_box_pack_start (GTK_BOX (hbox), pixmap, FALSE, FALSE, 0);
	new->priv->frame = gtk_frame_new (_("Preview"));
	gtk_widget_show (new->priv->frame);
	gtk_box_pack_start (GTK_BOX (hbox), new->priv->frame, TRUE, TRUE, 0);

	/* Second hbox */
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (new)->vbox), hbox,
			    TRUE, TRUE, 0);
	check = gtk_check_button_new_with_label (_("Refresh every "));
	gtk_widget_show (check);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), FALSE);
	gtk_signal_connect (GTK_OBJECT (check), "toggled",
			    GTK_SIGNAL_FUNC (on_toggle_toggled), new);
	gtk_box_pack_start (GTK_BOX (hbox), check, TRUE, FALSE, 0);
	new->priv->adjustment = gtk_adjustment_new (10.0, 1.0, 60.0,
						    1.0, 10.0, 1.0);
	gtk_signal_connect (new->priv->adjustment, "value_changed", 
			    GTK_SIGNAL_FUNC (on_adjustment_value_changed), new);
	new->priv->spin = gtk_spin_button_new (
			GTK_ADJUSTMENT (new->priv->adjustment), 1.0, 0);
	gtk_widget_show (new->priv->spin);
	gtk_box_pack_start (GTK_BOX (hbox), new->priv->spin, TRUE, FALSE, 0);
	label = gtk_label_new (_(" seconds"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

	/* Are previews supported? */
	if (camera->abilities->operations & GP_OPERATION_CAPTURE_PREVIEW) {
		do_capture (new);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
	} else {
		label = gtk_label_new (_("Unsupported"));
		gtk_widget_show (label);
		gtk_container_add (GTK_CONTAINER (new->priv->frame), label);
		gtk_widget_set_sensitive (check, FALSE);
		gtk_widget_set_sensitive (new->priv->spin, FALSE);
	}

	return (GNOME_DIALOG (new));
}

E_MAKE_TYPE (gnocam_capture, "GnoCamCapture", GnoCamCapture, gnocam_capture_class_init, gnocam_capture_init, PARENT_TYPE)

