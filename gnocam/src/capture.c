#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include <gconf/gconf-client.h>
#include <bonobo.h>
#include <bonobo/bonobo-stream-memory.h>
#include <libgnomevfs/gnome-vfs.h>
#include <tree.h>
#include "gnocam.h"
#include "capture.h"
#include "file-operations.h"
#include "cameras.h"
#include "preferences.h"

/**********************/
/* External Variables */
/**********************/

extern GtkWindow*	main_window;

/**************/
/* Prototypes */
/**************/

void on_capture_close_activate      	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
void on_capture_save_activate       	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
void on_capture_save_as_activate    	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
void on_capture_refresh_activate    	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);

void on_capture_capture_video_activate	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
void on_capture_capture_image_activate  (BonoboUIComponent* component, gpointer user_data, const gchar* cname);
void on_capture_capture_preview_activate(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
void on_capture_capture_activate 	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);

void on_duration_button_ok_clicked	(GtkButton* button, gpointer user_data);
void on_duration_button_cancel_clicked	(GtkButton* button, gpointer user_data);

/*************/
/* Callbacks */
/*************/

void
on_duration_button_ok_clicked (GtkButton* button, gpointer user_data)
{
        GtkAdjustment*          adjustment;
        Camera*                 camera;
	BonoboUIComponent*	component;

	g_return_if_fail (component = gtk_object_get_data (GTK_OBJECT (button), "component"));
	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (component), "camera"));
        g_return_if_fail (adjustment = gtk_range_get_adjustment (GTK_RANGE (gtk_object_get_data (GTK_OBJECT (button), "hscale"))));

        /* Prepare the video. */
	gtk_object_set_data (GTK_OBJECT (component), "duration", GINT_TO_POINTER ((int) adjustment->value));

        /* Capture. */
	on_capture_capture_activate (component, GINT_TO_POINTER (GP_CAPTURE_VIDEO), NULL);

        /* Clean up. */
        gtk_widget_destroy (gtk_object_get_data (GTK_OBJECT (button), "messagebox"));
}

void
on_duration_button_cancel_clicked (GtkButton* button, gpointer user_data)
{
        gtk_widget_destroy (gtk_object_get_data (GTK_OBJECT (button), "messagebox"));
}

void
on_capture_capture_video_activate (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
        GladeXML*               xml_duration;
        GladeXML*               xml_hscale;
        GnomeMessageBox*        messagebox;

        /* Ask for duration. */
        g_assert ((xml_duration = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "duration_messagebox")));
        glade_xml_signal_autoconnect (xml_duration);

        //FIXME: As soon libglade is fixed, delete this code.
        /* This is annoying: The hscale doesn't get displayed... We have to create it manually. */
        g_assert ((xml_hscale = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "duration_hscale")));
        g_assert ((messagebox = GNOME_MESSAGE_BOX (glade_xml_get_widget (xml_duration, "duration_messagebox"))));
        gtk_container_add (GTK_CONTAINER ((GNOME_DIALOG (messagebox))->vbox), glade_xml_get_widget (xml_hscale, "duration_hscale"));

        /* Store some data. */
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_duration, "duration_button_ok")), "messagebox", messagebox);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_duration, "duration_button_ok")), "hscale",  glade_xml_get_widget (xml_hscale, "duration_hscale"));
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_duration, "duration_button_cancel")), "messagebox", messagebox);

	/* Ref the component. */
	gtk_object_ref (GTK_OBJECT (component));
	gtk_object_set_data_full (GTK_OBJECT (glade_xml_get_widget (xml_duration, "duration_button_ok")), "component", component, (GtkDestroyNotify) gtk_object_unref);
}

void
on_capture_capture_image_activate (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	on_capture_capture_activate (component, GINT_TO_POINTER (GP_CAPTURE_IMAGE), cname);
}

void
on_capture_capture_preview_activate (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	on_capture_capture_activate (component, GINT_TO_POINTER (GP_CAPTURE_PREVIEW), cname);
}

void
on_capture_capture_activate (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
        gint                    result;
        gchar*                  tmp;
        Camera*                 camera;
        CameraFile*             file;
        CameraFile*             old_file;
        CameraCaptureInfo       info;
        CORBA_Environment       ev;
        Bonobo_Control          control;
        GtkWidget*              widget;
        GnomeVFSURI*            uri;

        g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (component), "camera"));

        /* Prepare the image. */
        file = gp_file_new ();

        info.type = GPOINTER_TO_INT (user_data);
	gtk_object_set_data (GTK_OBJECT (component), "type", user_data);
	if (info.type == GP_CAPTURE_VIDEO) info.duration = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (component), "duration"));

        /* Capture. */
        if ((result = gp_camera_capture (camera, file, &info)) == GP_OK) {
                if ((old_file = gtk_object_get_data (GTK_OBJECT (component), "file"))) gp_file_unref (old_file);
                gtk_object_set_data_full (GTK_OBJECT (component), "file", file, (GtkDestroyNotify) gp_file_unref);

                /* Init exception. */
                CORBA_exception_init (&ev);

                /* Destroy old viewers. */
                if ((widget = bonobo_window_get_contents (gtk_object_get_data (GTK_OBJECT (component), "window")))) gtk_widget_destroy (widget);

//FIXME: Moniker can not yet resolve "camera:".

                /* Get a control. */
                tmp = g_strdup_printf ("file:%s/%s", g_get_tmp_dir (), file->name);
                uri = gnome_vfs_uri_new (tmp);
                camera_file_save (file, uri);
                gnome_vfs_uri_unref (uri);
                control = bonobo_get_object (tmp, "IDL:Bonobo/Control:1.0", &ev);
                g_free (tmp);
                if (BONOBO_EX (&ev)) {
                        tmp = g_strdup_printf (_("Could not get any widget for\ndisplaying the captured data!\n(%s)"), bonobo_exception_get_text (&ev));
                        gnome_error_dialog_parented (tmp, main_window);
                        g_free (tmp);
                } else {
                        gtk_widget_show (widget = bonobo_widget_new_control_from_objref (control, bonobo_ui_component_get_container (component)));
                        bonobo_window_set_contents (gtk_object_get_data (GTK_OBJECT (component), "window"), widget);
                }

                /* Free exception. */
                CORBA_exception_free (&ev);

        } else {
                tmp = g_strdup_printf (_("Could not get capture from camera!\n(%s)"), gp_camera_result_as_string (camera, result));
                gnome_error_dialog_parented (tmp, main_window);
                g_free (tmp);
                gp_file_unref (file);
        }
}

void
on_capture_close_activate (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	gtk_widget_destroy (gtk_object_get_data (GTK_OBJECT (component), "window"));
}

void
on_capture_save_activate (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
        CameraFile*             file;
        GConfValue*             value;
        gchar*                  filename;
        GnomeVFSURI*            uri;

        if ((file = gtk_object_get_data (GTK_OBJECT (component), "file"))) {
                g_return_if_fail ((value = gconf_client_get (gconf_client_get_default (), "/apps/" PACKAGE "/prefix", NULL)));
                g_return_if_fail (value->type == GCONF_VALUE_STRING);
                filename = g_strdup_printf ("%s/%s", gconf_value_get_string (value), file->name);
                uri = gnome_vfs_uri_new (filename);
                g_free (filename);
                camera_file_save (file, uri);
                gnome_vfs_uri_unref (uri);
        }
}

void
on_capture_save_as_activate (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	if (gtk_object_get_data (GTK_OBJECT (component), "file")) camera_file_save (gtk_object_get_data (GTK_OBJECT (component), "file"), NULL);
}

void
on_capture_refresh_activate (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	on_capture_capture_activate (component, gtk_object_get_data (GTK_OBJECT (component), "type"), cname);
}

/*************/
/* Functions */
/*************/

GtkWidget*
capture_new (Camera* camera, CameraCaptureType type)
{
	xmlDocPtr		doc;
	xmlNodePtr		node, command, node_child;
	xmlNsPtr		ns;
	GtkWidget*		window;
	BonoboUIComponent*	component;
	BonoboUIContainer*	container;
	Bonobo_UIContainer	corba_container;
	BonoboUIVerb		verb [] = {
		BONOBO_UI_VERB ("Refresh", on_capture_refresh_activate), 
		BONOBO_UI_VERB ("Save", on_capture_save_activate),
		BONOBO_UI_VERB ("SaveAs", on_capture_save_as_activate),
		BONOBO_UI_VERB ("CapturePreview", on_capture_capture_preview_activate), 
		BONOBO_UI_VERB ("CaptureImage", on_capture_capture_image_activate),
		BONOBO_UI_VERB ("CaptureVideo", on_capture_capture_video_activate),
		BONOBO_UI_VERB ("Close", on_capture_close_activate),
		BONOBO_UI_UNSAFE_VERB ("Exit", gtk_main_quit),
		BONOBO_UI_UNSAFE_VERB ("Preferences", preferences),
		BONOBO_UI_VERB ("About", on_about_activate),
		BONOBO_UI_VERB_END};
	gchar*			tmp;
	gint			i;
	CameraWidget*		window_camera = NULL;
	CORBA_Environment	ev;

        g_return_val_if_fail (camera, NULL);
	g_return_val_if_fail (type & (GP_CAPTURE_IMAGE | GP_CAPTURE_PREVIEW | GP_CAPTURE_VIDEO), NULL);

	/* Create the interface. */
	gtk_widget_show (window = bonobo_window_new ("capture", "capture"));
	gtk_widget_ref (window);
	container = bonobo_ui_container_new ();
	corba_container = bonobo_object_corba_objref (BONOBO_OBJECT (container));
	gtk_object_set_data (GTK_OBJECT (window), "container", corba_container);
	bonobo_ui_container_set_win (container, BONOBO_WINDOW (window));
	component = bonobo_ui_component_new ("capture");
	gtk_object_set_data (GTK_OBJECT (component), "window", window);
	bonobo_ui_component_set_container (component, corba_container);
	bonobo_ui_component_add_verb_list_with_data (component, verb, window);
	bonobo_ui_util_set_ui (component, GNOCAM_DATADIR, "gnocam-capture.xml", "capture");

	/* Display the menu items for capture. */
	if (camera->abilities->capture & GP_CAPTURE_PREVIEW) bonobo_ui_component_set_prop (component, "/menu/File/CapturePreview", "hidden", "0", NULL);
	else bonobo_ui_component_set_prop (component, "/menu/File/CapturePreview", "hidden", "1", NULL);
	if (camera->abilities->capture & GP_CAPTURE_VIDEO) bonobo_ui_component_set_prop (component, "/menu/File/CaptureVideo", "hidden", "0", NULL);
	else bonobo_ui_component_set_prop (component, "/menu/File/CaptureVideo", "hidden", "1", NULL);
	if (camera->abilities->capture & GP_CAPTURE_IMAGE) bonobo_ui_component_set_prop (component, "/menu/File/CaptureImage", "hidden", "0", NULL);
	else bonobo_ui_component_set_prop (component, "/menu/File/CaptureImage", "hidden", "1", NULL);

	/* Ref the camera for the component. */
	gp_camera_ref (camera);
	gtk_object_set_data_full (GTK_OBJECT (component), "camera", camera, (GtkDestroyNotify) gp_camera_unref);

        /* Capture. */
	switch (type) {
	case GP_CAPTURE_PREVIEW:
	case GP_CAPTURE_IMAGE:
		on_capture_capture_activate (component, GINT_TO_POINTER (type), NULL);
		break;
	case GP_CAPTURE_VIDEO:
		on_capture_capture_video_activate (component, NULL, NULL);
		break;
	default:
		break;
	}

	return (window);
}




