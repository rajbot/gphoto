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
#include "preview.h"
#include "file-operations.h"
#include "cameras.h"
#include "utils.h"
#include "preferences.h"

/**********************/
/* External Variables */
/**********************/

extern GConfClient* 	gconf_client;
extern GtkWindow*	main_window;

/**************/
/* Prototypes */
/**************/

void on_preview_close_activate      (GtkWidget* widget, gpointer user_data);
void on_preview_save_activate       (GtkWidget* widget, gpointer user_data);
void on_preview_save_as_activate    (GtkWidget* widget, gpointer user_data);
void on_preview_refresh_activate    (GtkWidget* widget, gpointer user_data);

void on_preview_capture_image_activate 	(GtkWidget* widget, gpointer user_data);
void on_preview_capture_video_activate 	(GtkWidget* widget, gpointer user_data);

void preview_refresh 	(GtkWidget* preview);
void preview_save 	(GtkWidget* preview);
void preview_save_as 	(GtkWidget* preview);

/*************/
/* Callbacks */
/*************/

void
on_preview_capture_image_activate (GtkWidget* widget, gpointer user_data)
{
	capture_image (gtk_object_get_data (GTK_OBJECT (user_data), "camera"));
}

void
on_preview_capture_video_activate (GtkWidget* widget, gpointer user_data)
{
	capture_video (gtk_object_get_data (GTK_OBJECT (user_data), "camera"));
}

void
on_preview_close_activate (GtkWidget* widget, gpointer user_data)
{
	gtk_widget_destroy (GTK_WIDGET (user_data));
}

void
on_preview_save_activate (GtkWidget* widget, gpointer user_data)
{
        preview_save (GTK_WIDGET (user_data));
}

void
on_preview_save_as_activate (GtkWidget* widget, gpointer user_data)
{
        preview_save_as (GTK_WIDGET (user_data));
}

void
on_preview_refresh_activate (GtkWidget* widget, gpointer user_data)
{
        preview_refresh (GTK_WIDGET (user_data));
}

/*************/
/* Functions */
/*************/

void
preview_refresh (GtkWidget* preview)
{
	gint			result;
	gchar*			tmp;
	Camera*			camera;
        CameraFile*             file;
        CameraFile*             old_file;
        CameraCaptureInfo       info;
	CORBA_Environment	ev;
	Bonobo_UIContainer	corba_container;
	Bonobo_Control		control;
	GtkWidget*		widget;
	GnomeVFSURI*		uri;

	g_return_if_fail (preview);
	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (preview), "camera"));
	g_return_if_fail (corba_container = gtk_object_get_data (GTK_OBJECT (preview), "container"));

        /* Prepare the image. */
        file = gp_file_new ();
        info.type = GP_CAPTURE_PREVIEW;
        info.duration = 0;

        /* Capture. */
        if ((result = gp_camera_capture (camera, file, &info)) == GP_OK) {
		if ((old_file = gtk_object_get_data (GTK_OBJECT (preview), "file"))) gp_file_unref (old_file);
		gtk_object_set_data_full (GTK_OBJECT (preview), "file", file, (GtkDestroyNotify) gp_file_unref);
		
		/* Init exception. */
		CORBA_exception_init (&ev);

		/* Destroy old viewers. */
		widget = bonobo_window_get_contents (BONOBO_WINDOW (preview));
		if (widget) gtk_widget_destroy (widget);

//FIXME: Moniker can not yet resolve "camera:".

		/* Get a control. */
		tmp = g_strdup_printf ("file:%s/%s", g_get_tmp_dir (), file->name);
		uri = gnome_vfs_uri_new (tmp);
		camera_file_save (file, uri);
		gnome_vfs_uri_unref (uri);
		control = bonobo_get_object (tmp, "IDL:Bonobo/Control:1.0", &ev);
		g_free (tmp);
		if (BONOBO_EX (&ev)) {
			tmp = g_strdup_printf (_("Could not get any widget for\ndisplaying the preview!\n(%s)"), bonobo_exception_get_text (&ev));
			gnome_error_dialog_parented (tmp, main_window);
			g_free (tmp);
		} else {
			gtk_widget_show (widget = bonobo_widget_new_control_from_objref (control, corba_container));
			bonobo_window_set_contents (BONOBO_WINDOW (preview), widget);
		}
		
		/* Free exception. */
		CORBA_exception_free (&ev);

        } else {
                tmp = g_strdup_printf (_("Could not get preview from camera!\n(%s)"), gp_camera_result_as_string (camera, result));
		gnome_error_dialog_parented (tmp, main_window);
		g_free (tmp);
                gp_file_unref (file);
        }
}

void
preview_save (GtkWidget* preview)
{
	CameraFile*		file;
	GConfValue*		value;
	gchar*			filename;
	GnomeVFSURI*		uri;

	g_return_if_fail (preview);

        if ((file = gtk_object_get_data (GTK_OBJECT (preview), "file"))) {
		g_return_if_fail ((value = gconf_client_get (gconf_client, "/apps/" PACKAGE "/prefix", NULL)));
		g_return_if_fail (value->type == GCONF_VALUE_STRING);
		filename = g_strdup_printf ("%s/%s", gconf_value_get_string (value), file->name);
		uri = gnome_vfs_uri_new (filename);
		g_free (filename);
		camera_file_save (file, uri);
		gnome_vfs_uri_unref (uri);
        }
}

void
preview_save_as (GtkWidget* preview)
{
	if (gtk_object_get_data (GTK_OBJECT (preview), "file")) camera_file_save (gtk_object_get_data (GTK_OBJECT (preview), "file"), NULL);
}

GtkWidget*
preview_new (Camera* camera)
{
	xmlDocPtr		doc;
	xmlNodePtr		node, command, node_child;
	xmlNsPtr		ns;
	GtkWidget*		window;
	BonoboUIComponent*	component;
	BonoboUIContainer*	container;
	Bonobo_UIContainer	corba_container;
	BonoboUIVerb		verb [] = {
		BONOBO_UI_UNSAFE_VERB ("Refresh", on_preview_refresh_activate), 
		BONOBO_UI_UNSAFE_VERB ("Save", on_preview_save_activate),
		BONOBO_UI_UNSAFE_VERB ("SaveAs", on_preview_save_as_activate),
		BONOBO_UI_UNSAFE_VERB ("CaptureImage", on_preview_capture_image_activate),
		BONOBO_UI_UNSAFE_VERB ("CaptureVideo", on_preview_capture_video_activate),
		BONOBO_UI_UNSAFE_VERB ("Close", on_preview_close_activate),
		BONOBO_UI_UNSAFE_VERB ("Exit", gtk_main_quit),
		BONOBO_UI_UNSAFE_VERB ("Preferences", preferences),
		BONOBO_UI_UNSAFE_VERB ("About", on_about_activate),
		BONOBO_UI_VERB_END};
	gchar*			tmp;
	gint			i;
	CameraWidget*		window_camera = NULL;
	CORBA_Environment	ev;

        g_return_val_if_fail (camera, NULL);

	/* Create the interface. */
	window = bonobo_window_new ("Preview", "Preview");
	container = bonobo_ui_container_new ();
	corba_container = bonobo_object_corba_objref (BONOBO_OBJECT (container));
	bonobo_ui_container_set_win (container, BONOBO_WINDOW (window));
	component = bonobo_ui_component_new ("Preview");
	bonobo_ui_component_set_container (component, corba_container);
	bonobo_ui_component_add_verb_list_with_data (component, verb, window);
	bonobo_ui_util_set_ui (component, "", "gnocam-preview.xml", "Preview");
	gtk_widget_show_all (window);

        /* Store some data. */
	gtk_object_set_data (GTK_OBJECT (window), "container", corba_container);

	/* Ref the camera for the preview. */
	gp_camera_ref (camera);
	gtk_object_set_data_full (GTK_OBJECT (window), "camera", camera, (GtkDestroyNotify) gp_camera_unref);

	/* Ref the camera for the component. */
	gp_camera_ref (camera);
	gtk_object_set_data_full (GTK_OBJECT (component), "camera", camera, (GtkDestroyNotify) gp_camera_unref);

        /* Get a preview. */
        preview_refresh (window);

	/* Create the "camera properties" menu item. */
	if (camera->abilities->config && (gp_camera_config_get (camera, &window_camera) == GP_OK)) {
		doc = xmlNewDoc ("1.0");
		ns = xmlNewGlobalNs (doc, "xxx", "xxx");
		xmlDocSetRootElement (doc, node = xmlNewNode (ns, "Root"));
		xmlAddChild (node, command = xmlNewNode (ns, "commands"));
		xmlAddChild (node, node_child = xmlNewNode (ns, "menu"));
		xmlAddChild (node_child, node = xmlNewNode (ns, "submenu"));
		xmlSetProp (node, "name", "Edit");
		xmlSetProp (node, "_label", _("_Edit"));
		xmlSetProp (node, "_tip", _("Edit"));
		popup_prepare (component, window_camera, node, command, ns);
		xmlDocDumpMemory (doc, (xmlChar**) &tmp, &i);
		xmlFreeDoc (doc);
		CORBA_exception_init (&ev);
		bonobo_ui_component_set_translate (component, "/", tmp, &ev);
		CORBA_exception_free (&ev);
		g_free (tmp);
		popup_fill (component, "/menu/Edit", window_camera, window_camera, TRUE);
	}

	return (window);
}




