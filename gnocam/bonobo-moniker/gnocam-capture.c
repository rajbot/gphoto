#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-capture.h"

#include <bonobo/bonobo-stream-memory.h>
#include <gal/util/e-util.h>

#define PARENT_TYPE bonobo_window_get_type ()
static BonoboWindowClass* parent_class = NULL;

struct _GnoCamCapturePrivate {
	Camera*			camera;
	Bonobo_UIContainer	container;

	CameraCaptureType	type;
	gint			duration;
};

#define GNOCAM_CAPTURE_UI 												\
"<Root>"														\
"  <menu>"														\
"    <submenu name=\"File\" _label=\"_File\">"										\
"      <menuitem name=\"CapturePreview\" verb=\"\" _label=\"Capture Preview\"/>"					\
"      <menuitem name=\"CaptureImage\" verb=\"\" _label=\"Capture Image\"/>"						\
"      <menuitem name=\"CaptureVideo\" verb=\"\" _label=\"Capture Video\"/>"						\
"      <separator/>"													\
"      <menuitem name=\"Close\" verb=\"\" _label=\"_Close\" pixtype=\"stock\" pixname=\"Close\"/>"			\
"    </submenu>"													\
"    <placeholder name=\"View\"/>"											\
"    <submenu name=\"Help\" _label=\"_Help\">"										\
"      <menuitem name=\"About\" _label=\"_About\" pixtype=\"stock\" pixname=\"About\"/>"				\
"    </submenu>"													\
"  </menu>"														\
"  <dockitem name=\"Toolbar\" homogenous=\"0\" look=\"icons\">"								\
"    <toolitem name=\"Refresh\" verb=\"\" _label=\"Refresh\" _tip=\"Refresh\" pixtype=\"stock\" pixname=\"Refresh\"/>"	\
"  </dockitem>"														\
"</Root>"


/********************/
/* Helper functions */
/********************/

static void 
do_capture (GnoCamCapture* capture)
{
	GtkWidget*		widget;
	CameraFile*		file;
	CameraCaptureInfo	info;
	gint			result;
	gchar*                  oaf_requirements;
	CORBA_Environment	ev;
	OAF_ActivationID        ret_id;
	BonoboStream*		stream;
	Bonobo_Unknown		object;
	Bonobo_Control		control;
	Bonobo_Persist		persist;
	Bonobo_Stream		corba_stream;

        /* Prepare the image. */
        file = gp_file_new ();
        info.type = capture->priv->type;
	info.duration = capture->priv->duration;

        /* Capture. */
        result = gp_camera_capture (capture->priv->camera, file, &info);
	if (result != GP_OK) {
		g_warning ("Could not capture! (%s)", gp_camera_result_as_string (capture->priv->camera, result));
		gp_file_unref (file);
		return;
	}

	/* Init exception */
	CORBA_exception_init (&ev);

	oaf_requirements = g_strdup_printf (
		"bonobo:supported_mime_types.has ('%s') AND "
		"repo_ids.has ('IDL:Bonobo/Control:1.0') AND "
		"repo_ids.has ('IDL:Bonobo/PersistStream:1.0')", file->type);

	/* Activate the object */
        object = oaf_activate (oaf_requirements, NULL, 0, &ret_id, &ev);
        g_free (oaf_requirements);
        if (BONOBO_EX (&ev)) {
		g_warning ("Could not get object capable of handling file of type %s! (%s)", file->type, bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		gp_file_unref (file);
                return;
        }
	g_return_if_fail (object);

	/* Create the stream */
	stream = bonobo_stream_mem_create (file->data, file->size, TRUE, FALSE);
	corba_stream = bonobo_object_corba_objref (BONOBO_OBJECT (stream));

	/* Get the persist stream interface */
         persist = Bonobo_Unknown_queryInterface (object, "IDL:Bonobo/PersistStream:1.0", &ev);
         if (BONOBO_EX (&ev)) {
		g_warning ("Could not get interface! (%s)", bonobo_exception_get_text (&ev));
		gp_file_unref (file);
		CORBA_exception_free (&ev);
                return;
        }
        g_return_if_fail (persist);

        /* Load the persist stream */
        Bonobo_PersistStream_load (persist, corba_stream, (const Bonobo_Persist_ContentType) file->type, &ev);
	gp_file_unref (file);
        if (BONOBO_EX (&ev)) {
		g_warning ("Could not load stream! (%s)", bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return;
	}

	bonobo_object_release_unref (persist, &ev);
	bonobo_object_unref (BONOBO_OBJECT (stream));

	control = bonobo_moniker_util_qi_return (object, "IDL:Bonobo/Control:1.0", &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not get control!");
		CORBA_exception_free (&ev);
		return;
	}
	CORBA_exception_free (&ev);

	/* Display! */
	if ((widget = bonobo_window_get_contents (BONOBO_WINDOW (capture)))) gtk_widget_destroy (widget);
	gtk_widget_show (widget = bonobo_widget_new_control_from_objref (control, capture->priv->container));
        bonobo_window_set_contents (BONOBO_WINDOW (capture), widget);
}

/*************/
/* Callbacks */
/*************/

static void
on_duration_clicked (GnomeDialog* dialog, gint button_number)
{
	g_warning ("Clicked %i", button_number);

	if (button_number == 0) {
		GnoCamCapture*	capture;
		GtkAdjustment*	adjustment;

		capture = GNOCAM_CAPTURE (gtk_object_get_data (GTK_OBJECT (dialog), "capture"));
		adjustment = GTK_ADJUSTMENT (gtk_object_get_data (GTK_OBJECT (dialog), "adjustment"));
		capture->priv->type = GP_CAPTURE_VIDEO;
		capture->priv->duration = adjustment->value;

		do_capture (capture);
	}

	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
on_capture_video_activate (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCapture*	capture;
	GtkWidget*	new;
	GtkWidget*	widget;

	capture = GNOCAM_CAPTURE (user_data);

	new = gnome_message_box_new (_("How long should the video be (in seconds)?"), GNOME_MESSAGE_BOX_QUESTION);
	gtk_signal_connect (GTK_OBJECT (new), "clicked", GTK_SIGNAL_FUNC (on_duration_clicked), new);
	gtk_object_set_data (GTK_OBJECT (new), "capture", capture);

	gtk_widget_show (widget = gtk_spin_button_new (NULL, 2, 0));
        gtk_container_add (GTK_CONTAINER ((GNOME_DIALOG (new))->vbox), widget);
	gtk_object_set_data (GTK_OBJECT (new), "adjustment", gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (widget)));
}

static void
on_capture_image_activate (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCapture*	capture;

	capture = GNOCAM_CAPTURE (user_data);
	capture->priv->type = GP_CAPTURE_IMAGE;
	
	do_capture (capture);
}

static void
on_capture_preview_activate (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCapture*  capture;

	capture = GNOCAM_CAPTURE (user_data);
	capture->priv->type = GP_CAPTURE_PREVIEW;

	do_capture (capture);
}

static void
on_capture_refresh_activate (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCapture*	capture;

	capture = GNOCAM_CAPTURE (user_data);

	do_capture (capture);
}

static void
on_capture_close_activate (BonoboUIComponent* component, gpointer user_data, const gchar* name)
{
	GnoCamCapture*	capture;

	capture = GNOCAM_CAPTURE (user_data);

	gtk_widget_destroy (GTK_WIDGET (capture));
}

/***********************/
/* Bonobo-Window stuff */
/***********************/

static void
gnocam_capture_destroy (GtkObject* object)
{
        GnoCamCapture*  capture;

        capture = GNOCAM_CAPTURE (object);

        gp_camera_unref (capture->priv->camera);
        g_free (capture->priv);

        (*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gnocam_capture_class_init (GnoCamCaptureClass* klass)
{
	GtkObjectClass*	object_class;

	object_class = (GtkObjectClass*) klass;
	object_class->destroy = gnocam_capture_destroy;

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_capture_init (GnoCamCapture* capture)
{
	capture->priv = g_new (GnoCamCapturePrivate, 1);
	capture->priv->camera = NULL;
}

GnoCamCapture*
gnocam_capture_new (Camera* camera, CameraCaptureType type)
{
	GnoCamCapture*		new;
	BonoboUIContainer*	container;
	BonoboUIComponent*	component;
	BonoboUIVerb		verb [] = {
		BONOBO_UI_VERB ("Refresh", on_capture_refresh_activate), 
		BONOBO_UI_VERB ("CapturePreview", on_capture_preview_activate), 
		BONOBO_UI_VERB ("CaptureImage", on_capture_image_activate),
		BONOBO_UI_VERB ("CaptureVideo", on_capture_video_activate),
		BONOBO_UI_VERB ("Close", on_capture_close_activate),
		BONOBO_UI_VERB_END};

        g_return_val_if_fail (camera, NULL);

	new = gtk_type_new (gnocam_capture_get_type ());
	new = GNOCAM_CAPTURE (bonobo_window_construct (BONOBO_WINDOW (new), "GnoCamCapture", "GnoCam Capture"));
	new->priv->camera = camera;
	gp_camera_ref (camera);

	gtk_window_set_default_size (GTK_WINDOW (new), 550, 550);
	
	/* Create the container */
	container = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (container, BONOBO_WINDOW (new));
	new->priv->container = bonobo_object_corba_objref (BONOBO_OBJECT (container));

	/* Create the menu */
	component = bonobo_ui_component_new ("capture");
	bonobo_ui_component_set_container (component, new->priv->container);
	bonobo_ui_component_add_verb_list_with_data (component, verb, new);
	bonobo_ui_component_set_translate (component, "/", GNOCAM_CAPTURE_UI, NULL);

	/* Display the menu items for capture. */
	if (camera->abilities->capture & GP_CAPTURE_PREVIEW) bonobo_ui_component_set_prop (component, "/menu/File/CapturePreview", "hidden", "0", NULL);
	else bonobo_ui_component_set_prop (component, "/menu/File/CapturePreview", "hidden", "1", NULL);
	if (camera->abilities->capture & GP_CAPTURE_VIDEO) bonobo_ui_component_set_prop (component, "/menu/File/CaptureVideo", "hidden", "0", NULL);
	else bonobo_ui_component_set_prop (component, "/menu/File/CaptureVideo", "hidden", "1", NULL);
	if (camera->abilities->capture & GP_CAPTURE_IMAGE) bonobo_ui_component_set_prop (component, "/menu/File/CaptureImage", "hidden", "0", NULL);
	else bonobo_ui_component_set_prop (component, "/menu/File/CaptureImage", "hidden", "1", NULL);

        /* Capture. */
	switch (type) {
	case GP_CAPTURE_PREVIEW:
		on_capture_preview_activate (component, new, NULL);
		break;
	case GP_CAPTURE_IMAGE:
		on_capture_image_activate (component, new, NULL);
		break;
	case GP_CAPTURE_VIDEO:
		on_capture_video_activate (component, new, NULL);
		break;
	default:
		break;
	}

	return (new);
}

E_MAKE_TYPE (gnocam_capture, "GnoCamCapture", GnoCamCapture, gnocam_capture_class_init, gnocam_capture_init, PARENT_TYPE)

