#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-capture.h"

#include <bonobo/bonobo-stream-memory.h>
#include <gal/util/e-util.h>

#include "utils.h"
#include "gnocam-configuration.h"

#define PARENT_TYPE BONOBO_TYPE_WINDOW
static BonoboWindowClass* parent_class = NULL;

struct _GnoCamCapturePrivate {
	BonoboUIContainer*	container;
	BonoboUIComponent*	component;

	GConfClient*		client;

	Camera*			camera;
	CameraWidget*		configuration;

	CameraCaptureType	type;
	gint			duration;
};

#define GNOCAM_CAPTURE_UI 													\
"<Root>"															\
"  <menu>"															\
"    <submenu name=\"File\" _label=\"_File\">"											\
"      <placeholder name=\"FileOperations\"/>"											\
"      <placeholder name=\"System\" delimit=\"top\">"										\
"        <menuitem name=\"Close\" verb=\"\" _label=\"_Close\" pixtype=\"stock\" pixname=\"Close\" accel=\"*Control*w\"/>"	\
"      </placeholder>"														\
"    </submenu>"														\
"    <submenu name=\"Camera\" _label=\"Camera\">"                         							\
"      <menuitem name=\"Manual\" _label=\"Manual\" verb=\"\" pixtype=\"stock\" pixname=\"Book Open\"/>"				\
"      <placeholder name=\"CaptureOperations\" delimit=\"top\"/>"								\
"      <placeholder name=\"Configuration\" delimit=\"top\"/>"									\
"    </submenu>"                                                          							\
"    <submenu name=\"Edit\" _label=\"_Edit\">"                                                                          	\
"       <placeholder/>"                                                                                                 	\
"       <menuitem name=\"BonoboCustomize\" verb=\"\" _label=\"Customi_ze...\" pos=\"bottom\"/>"                         	\
"    </submenu>"                                                                                                        	\
"    <submenu name=\"View\" _label=\"_View\"/>"											\
"    <submenu name=\"Help\" _label=\"_Help\">"											\
"      <menuitem name=\"About\" _label=\"_About\" pixtype=\"stock\" pixname=\"About\"/>"					\
"    </submenu>"														\
"  </menu>"															\
"  <dockitem name=\"Toolbar\" homogenous=\"0\" look=\"icons\">"									\
"    <toolitem name=\"Refresh\" verb=\"\" _label=\"Refresh\" _tip=\"Refresh\" pixtype=\"stock\" pixname=\"Refresh\"/>"		\
"  </dockitem>"															\
"</Root>"

#define GNOCAM_CAPTURE_UI_CONFIGURATION											\
"<placeholder name=\"Configuration\">"											\
"  <menuitem name=\"Configuration\" _label=\"Configuration\" verb=\"\" pixtype=\"stock\" pixname=\"Properties\"/>"	\
"</placeholder>"

#define GNOCAM_CAPTURE_UI_IMAGE                                                 \
"<placeholder name=\"CaptureOperations\">"                                      \
"  <menuitem name=\"CaptureImage\" _label=\"Capture Image\" verb=\"\"/>"        \
"</placeholder>"

#define GNOCAM_CAPTURE_UI_PREVIEW						\
"<placeholder name=\"CaptureOperations\">"                                      \
"  <menuitem name=\"CapturePreview\" _label=\"Capture Preview\" verb=\"\"/>"    \
"</placeholder>"

#define GNOCAM_CAPTURE_UI_VIDEO							\
"<placeholder name=\"CaptureOperations\">"                                      \
"  <menuitem name=\"CaptureVideo\" _label=\"Capture Video\" verb=\"\"/>"        \
"</placeholder>"

/**************/
/* Prototypes */
/**************/

static void 	on_manual_clicked 		(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_capture_image_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_capture_video_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_capture_preview_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_capture_refresh_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_configuration_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_close_clicked		(BonoboUIComponent* component, gpointer user_data, const gchar* cname);

/********************/
/* Helper functions */
/********************/

static gint
create_menu (gpointer user_data)
{
	GnoCamCapture*	capture;

	capture = GNOCAM_CAPTURE (user_data);

        /* Create the component */
        capture->priv->component = bonobo_ui_component_new (PACKAGE "Capture");
        bonobo_ui_component_set_container (capture->priv->component, BONOBO_OBJREF (capture->priv->container));

        /* Create the menu */
        bonobo_ui_component_set_translate (capture->priv->component, "/", GNOCAM_CAPTURE_UI, NULL);
        bonobo_ui_component_add_verb (capture->priv->component, "Manual", on_manual_clicked, capture);
        bonobo_ui_component_add_verb (capture->priv->component, "Close", on_close_clicked, capture);
        bonobo_ui_component_add_verb (capture->priv->component, "Refresh", on_capture_refresh_clicked, capture);

        /* Camera Configuration? */
        if (capture->priv->camera->abilities->config) {
		bonobo_ui_component_set_translate (capture->priv->component, "/menu/Camera/Configuration", GNOCAM_CAPTURE_UI_CONFIGURATION, NULL);
		bonobo_ui_component_add_verb (capture->priv->component, "Configuration", on_configuration_clicked, capture);
        }

        /* Capture? */
        if (capture->priv->camera->abilities->capture & GP_CAPTURE_IMAGE) {
                bonobo_ui_component_set_translate (capture->priv->component, "/menu/Camera/CaptureOperations", GNOCAM_CAPTURE_UI_IMAGE, NULL);
                bonobo_ui_component_add_verb (capture->priv->component, "CaptureImage", on_capture_image_clicked, capture);
        }
        if (capture->priv->camera->abilities->capture & GP_CAPTURE_VIDEO) {
                bonobo_ui_component_set_translate (capture->priv->component, "/menu/Camera/CaptureOperations", GNOCAM_CAPTURE_UI_VIDEO, NULL);
                bonobo_ui_component_add_verb (capture->priv->component, "CaptureVideo", on_capture_video_clicked, capture);
        }
        if (capture->priv->camera->abilities->capture & GP_CAPTURE_PREVIEW) {
                bonobo_ui_component_set_translate (capture->priv->component, "/menu/Camera/CaptureOperations", GNOCAM_CAPTURE_UI_PREVIEW, NULL);
                bonobo_ui_component_add_verb (capture->priv->component, "CapturePreview", on_capture_preview_clicked, capture);
        }

	return (FALSE);
}

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
		g_warning (_("Could not capture!\n(%s)"), gp_camera_result_as_string (capture->priv->camera, result));
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
		g_warning (_("Could not get object capable of handling file of type %s!\n(%s)"), file->type, bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		gp_file_unref (file);
                return;
        }
	g_return_if_fail (object);

	/* Create the stream */
	stream = bonobo_stream_mem_create (file->data, file->size, TRUE, FALSE);
	gp_file_unref (file);
	corba_stream = BONOBO_OBJREF (stream);

	/* Get the persist stream interface */
         persist = Bonobo_Unknown_queryInterface (object, "IDL:Bonobo/PersistStream:1.0", &ev);
         if (BONOBO_EX (&ev)) {
	 	g_warning (_("Could not get 'PersistStream' interface!\n(%s)"), bonobo_exception_get_text (&ev));
		bonobo_object_unref (BONOBO_OBJECT (stream));
		CORBA_exception_free (&ev);
                return;
        }
        g_return_if_fail (persist);

        /* Load the persist stream */
        Bonobo_PersistStream_load (persist, corba_stream, (const Bonobo_Persist_ContentType) file->type, &ev);
	bonobo_object_unref (BONOBO_OBJECT (stream));
	bonobo_object_release_unref (persist, &ev);
        if (BONOBO_EX (&ev)) {
		g_warning (_("Could not load stream!\n(%s)"), bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return;
	}

	/* Get the control */
	control = Bonobo_Unknown_queryInterface (object, "IDL:Bonobo/Control:1.0", &ev);
	if (BONOBO_EX (&ev)) {
		g_warning (_("Could not get control!\n(%s)"), bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return;
	}

	/* Destroy old widget */
	if ((widget = bonobo_window_get_contents (BONOBO_WINDOW (capture)))) gtk_widget_unref (widget);

	/* Show new widget */
	widget = bonobo_widget_new_control_from_objref (control, BONOBO_OBJREF (capture->priv->container));
	gtk_widget_show (widget);
        bonobo_window_set_contents (BONOBO_WINDOW (capture), widget);

	Bonobo_Control_activate (control, TRUE, &ev);
	CORBA_exception_free (&ev);
}

/*************/
/* Callbacks */
/*************/

static void
on_window_size_request (GtkWidget* widget, GtkRequisition* requisition, gpointer user_data)
{
        GnoCamCapture*     capture;

        capture = GNOCAM_CAPTURE (user_data);

        gconf_client_set_int (capture->priv->client, "/apps/" PACKAGE "/width_capture", widget->allocation.width, NULL);
        gconf_client_set_int (capture->priv->client, "/apps/" PACKAGE "/height_capture", widget->allocation.height, NULL);
}

static void
on_configuration_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCapture*	capture;
	GtkWidget*	widget;

	capture = GNOCAM_CAPTURE (user_data);
	
	widget = gnocam_configuration_new (capture->priv->camera, NULL, NULL, GTK_WINDOW (capture));
	gtk_widget_show (widget);
}

static void
on_manual_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
        GnoCamCapture*  capture;
        gint            result;
        CameraText      manual;

        capture = GNOCAM_CAPTURE (user_data);

        result = gp_camera_manual (capture->priv->camera, &manual);
        if (result != GP_OK) {
		g_warning (_("Could not get camera manual: %s!"), gp_camera_result_as_string (capture->priv->camera, result));
		return;
        }

        g_message (manual.text);
}

static void
on_duration_clicked (GnomeDialog* dialog, gint button_number)
{
	if (button_number == 0) {
		GnoCamCapture*	capture;
		GtkAdjustment*	adjustment;

		capture = GNOCAM_CAPTURE (gtk_object_get_data (GTK_OBJECT (dialog), "capture"));
		adjustment = GTK_ADJUSTMENT (gtk_object_get_data (GTK_OBJECT (dialog), "adjustment"));
		capture->priv->type = GP_CAPTURE_VIDEO;
		capture->priv->duration = adjustment->value;

		do_capture (capture);
	}
}

static void
on_capture_video_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCapture*	capture;
	GtkWidget*	new;
	GtkWidget*	widget;
	GtkObject*	adjustment;

	capture = GNOCAM_CAPTURE (user_data);
	
	gtk_widget_show (new = gnome_message_box_new (_("How long should the video be (in seconds)?"), 
		GNOME_MESSAGE_BOX_QUESTION, GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_CANCEL, NULL));
	gtk_signal_connect (GTK_OBJECT (new), "clicked", GTK_SIGNAL_FUNC (on_duration_clicked), new);
	gtk_object_set_data (GTK_OBJECT (new), "capture", capture);

	adjustment = gtk_adjustment_new (0, 0, 99999, 1, 10, 1);
	gtk_widget_show (widget = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 2, 0));
        gtk_container_add (GTK_CONTAINER ((GNOME_DIALOG (new))->vbox), widget);
	gtk_object_set_data (GTK_OBJECT (new), "adjustment", adjustment);
}

static void
on_capture_image_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCapture*	capture;

	capture = GNOCAM_CAPTURE (user_data);
	capture->priv->type = GP_CAPTURE_IMAGE;
	
	do_capture (capture);
}

static void
on_capture_preview_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCapture*  capture;

	capture = GNOCAM_CAPTURE (user_data);
	capture->priv->type = GP_CAPTURE_PREVIEW;

	do_capture (capture);
}

static void
on_capture_refresh_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCapture*	capture;

	capture = GNOCAM_CAPTURE (user_data);

	do_capture (capture);
}

static void
on_close_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* name)
{
	GnoCamCapture*	capture;

	capture = GNOCAM_CAPTURE (user_data);

	gtk_widget_unref (GTK_WIDGET (capture));
}

/***********************/
/* Bonobo-Window stuff */
/***********************/

static void
gnocam_capture_destroy (GtkObject* object)
{
        GnoCamCapture*  capture;

        capture = GNOCAM_CAPTURE (object);

	bonobo_object_unref (BONOBO_OBJECT (capture->priv->component));
	bonobo_object_unref (BONOBO_OBJECT (capture->priv->container));

        gp_camera_unref (capture->priv->camera);
	if (capture->priv->configuration) gp_widget_unref (capture->priv->configuration);
	gtk_object_unref (GTK_OBJECT (capture->priv->client));
        g_free (capture->priv);

//	GTK_OBJECT_CLASS (parent_class)->destroy (object);
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
	capture->priv->configuration = NULL;
	capture->priv->camera = NULL;
}

GnoCamCapture*
gnocam_capture_new (Camera* camera, CameraCaptureType type, GConfClient* client, GtkWindow* window)
{
	GnoCamCapture*		new;
	gint			w, h;

        g_return_val_if_fail (camera, NULL);

	new = gtk_type_new (GNOCAM_TYPE_CAPTURE);
	new = GNOCAM_CAPTURE (bonobo_window_construct (BONOBO_WINDOW (new), PACKAGE "Capture", PACKAGE " - Capture"));
	gtk_signal_connect (GTK_OBJECT (new), "size_request", GTK_SIGNAL_FUNC (on_window_size_request), new);
	gp_camera_ref (new->priv->camera = camera);
	new->priv->type = type;
	gtk_object_ref (GTK_OBJECT (new->priv->client = client));
	gtk_window_set_transient_for (GTK_WINDOW (new), window);

	bonobo_ui_engine_config_set_path (bonobo_window_get_ui_engine (BONOBO_WINDOW (new)), "/" PACKAGE "/UIConf/capture");

	/* Create the container */
	new->priv->container = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (new->priv->container, BONOBO_WINDOW (new));

	/* Create the menu */
	gtk_idle_add (create_menu, new);

        /* Capture. */
	switch (type) {
	case GP_CAPTURE_PREVIEW:
		on_capture_preview_clicked (NULL, new, NULL);
		break;
	case GP_CAPTURE_IMAGE:
		on_capture_image_clicked (NULL, new, NULL);
		break;
	case GP_CAPTURE_VIDEO:
		on_capture_video_clicked (NULL, new, NULL);
		break;
	default:
		break;
	}

	/* Set the default settings */
        w = gconf_client_get_int (new->priv->client, "/apps/" PACKAGE "/width_capture", NULL);
        h = gconf_client_get_int (new->priv->client, "/apps/" PACKAGE "/height_capture", NULL);
	if (w + h == 0) gtk_window_set_default_size (GTK_WINDOW (new), 500, 500);
	else gtk_window_set_default_size (GTK_WINDOW (new), w, h);

	return (new);
}

E_MAKE_TYPE (gnocam_capture, "GnoCamCapture", GnoCamCapture, gnocam_capture_class_init, gnocam_capture_init, PARENT_TYPE)

