#include <config.h>
#include <gphoto2.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <bonobo.h>

#include <GnoCam.h>

#include "preferences.h"

/**********************/
/* External Variables */
/**********************/

extern GtkWindow*	main_window;

/**************/
/* Prototypes */
/**************/

void on_druidpagestandard_model_prepare			(GnomeDruidPage* page, GtkWidget* druid);
void on_druidpagestandard_port_prepare 			(GnomeDruidPage* page, GtkWidget* druid);
void on_druid_finish					(GnomeDruidPage* page, GtkWidget* druid);
void on_druid_cancel					(GnomeDruid* druid);

void on_button_camera_new_clicked 			(GtkButton* button);
void on_button_camera_delete_clicked 			(GtkButton* button);

void on_dialog_preferences_button_ok_clicked 		(GtkButton* button);

void on_radiobutton_debug_level_toggled (GtkToggleButton* toggle_button);

/*************/
/* Callbacks */
/*************/

void
on_druidpagestandard_model_prepare (GnomeDruidPage* page, GtkWidget* druid)
{
	gchar		buffer[1024];
	gchar*		model;
	GladeXML*	xml;
	gint		number_of_models, i;
	GList*		list = NULL;
	GtkWindow*	window;
	GtkEntry*	entry;
	GtkCombo*	combo;

	g_return_if_fail (xml = gtk_object_get_data (GTK_OBJECT (druid), "xml"));
	g_return_if_fail (entry = GTK_ENTRY (glade_xml_get_widget (xml, "entry_model")));
	g_return_if_fail (combo = GTK_COMBO (glade_xml_get_widget (xml, "combo_model")));
	g_return_if_fail (window = GTK_WINDOW (glade_xml_get_widget (xml, "window_druid")));

	/* Build model list. */
        if ((number_of_models = gp_camera_count ()) >= 0) {
                for (i = 0; i < number_of_models; i++) {
                        if (gp_camera_name (i, buffer) != GP_OK) strcpy (buffer, "?");
                        list = g_list_append (list, g_strdup (buffer));
                }
		if (!list) list = g_list_append (NULL, g_strdup (""));
		model = g_strdup (gtk_entry_get_text (entry));
                gtk_combo_set_popdown_strings (combo, list);
		gtk_entry_set_text (entry, model);
        } else {
                gchar* tmp = g_strdup_printf (_("Could not get number of supported models!\n(%s)"), gp_result_as_string (number_of_models));
                gnome_error_dialog_parented (tmp, window);
                g_free (tmp);
        }
}

void
on_druidpagestandard_port_prepare (GnomeDruidPage* page, GtkWidget* druid)
{
	GladeXML*	xml;
	GtkWindow*	window;
	GtkEntry*	entry_model;
	GtkEntry*	entry_port;
	GtkCombo*	combo;
	GList*		list = NULL;
	gint		i, result;
	CameraPortInfo	info;
	CameraAbilities	abilities;
	gchar*		port;
	
	g_return_if_fail (xml = gtk_object_get_data (GTK_OBJECT (druid), "xml"));
	g_return_if_fail (entry_model = GTK_ENTRY (glade_xml_get_widget (xml, "entry_model")));
	g_return_if_fail (entry_port = GTK_ENTRY (glade_xml_get_widget (xml, "entry_port")));
	g_return_if_fail (combo = GTK_COMBO (glade_xml_get_widget (xml, "combo_port")));
	g_return_if_fail (window = GTK_WINDOW (glade_xml_get_widget (xml, "window_druid")));
	
        if ((result = gp_camera_abilities_by_name (gtk_entry_get_text (entry_model), &abilities)) == GP_OK) {
                for (i = 0; i < gp_port_count_get (); i++) {
                        if ((result = gp_port_info_get (i, &info)) != GP_OK) {
                                gchar* tmp = g_strdup_printf (_("Could not get information about port number %i!\n(%s)"), i, gp_result_as_string (result));
                                gnome_error_dialog_parented (tmp, window);
                                g_free (tmp);
				continue;
                        }
                        if (    ((info.type == GP_PORT_SERIAL) && (SERIAL_SUPPORTED (abilities.port))) ||
                        	((info.type == GP_PORT_PARALLEL) && (PARALLEL_SUPPORTED (abilities.port))) ||
                                ((info.type == GP_PORT_USB) && (USB_SUPPORTED (abilities.port))) ||
                                ((info.type == GP_PORT_IEEE1394) && (IEEE1394_SUPPORTED (abilities.port))) ||
                                ((info.type == GP_PORT_NETWORK) && (NETWORK_SUPPORTED (abilities.port))))
                                list = g_list_append (list, g_strdup (info.name));
                }
                if (!list) list = g_list_append (NULL, g_strdup (""));
		port = g_strdup (gtk_entry_get_text (entry_port));
                gtk_combo_set_popdown_strings (combo, list);
		gtk_entry_set_text (entry_port, port);
        } else {
                gchar* tmp = g_strdup_printf (_("Could not get abilities for model '%s'!\n(%s)"), gtk_entry_get_text (entry_model), gp_result_as_string (result));
                gnome_error_dialog_parented (tmp, window);
                g_free (tmp);
        }
}

void
on_druid_finish (GnomeDruidPage* page, GtkWidget* druid)
{
	GConfClient*	client;
	GladeXML*	xml;
	gchar* 		model;
	gchar*		name;
	gchar*		port;
	gint		i;

	g_return_if_fail (xml = gtk_object_get_data (GTK_OBJECT (druid), "xml"));
	g_return_if_fail (model = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "entry_model"))));
	g_return_if_fail (name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "entry_name"))));
	g_return_if_fail (port = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "entry_port"))));
	g_return_if_fail (client = gconf_client_get_default ());

	for (i = 0; ; i++) {
		gchar* path = g_strdup_printf ("/apps/" PACKAGE "/camera/%i", i);
		if (!gconf_client_dir_exists (client, path, NULL)) {
			gchar* tmp;

			tmp = g_strconcat (path, "/port", NULL);
			gconf_client_set_string (client, tmp, port, NULL);
			g_free (tmp);

			tmp = g_strconcat (path, "/model", NULL);
			gconf_client_set_string (client, tmp, model, NULL);
			g_free (tmp);

			tmp = g_strconcat (path, "/name", NULL);
			gconf_client_set_string (client, tmp, name, NULL);
			g_free (tmp);

			g_free (path);
			break;
		}
		g_free (path);
	}

	gtk_widget_destroy (glade_xml_get_widget (xml, "window_druid"));
}

void
on_druid_cancel (GnomeDruid* druid)
{
	gtk_widget_destroy (glade_xml_get_widget (gtk_object_get_data (GTK_OBJECT (druid), "xml"), "window_druid"));
}

void
on_button_camera_new_clicked (GtkButton* button)
{
	GladeXML*	xml;

	/* Create the druid. */
	g_return_if_fail (xml = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "window_druid"));
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "druid")), "xml", xml);
	glade_xml_signal_autoconnect (xml);
}

void
on_button_camera_delete_clicked (GtkButton *button)
{
	GConfClient*	client = gconf_client_get_default ();
	GList*		selection;
	GtkCList*	clist;

	g_return_if_fail (clist = GTK_CLIST (glade_xml_get_widget (gtk_object_get_data (GTK_OBJECT (button), "xml"), "clist_cameras")));

	/* Remove the rows in the camera list. */
	while ((selection = g_list_first (clist->selection))) {
		gint	row = GPOINTER_TO_INT (selection->data);
		gchar*	tmp;
		
		tmp = g_strdup_printf ("/apps/" PACKAGE "/camera/%i", GPOINTER_TO_INT (gtk_clist_get_row_data (clist, row)));
		gconf_client_remove_dir (client, tmp, NULL);
		g_free (tmp);
		
		gtk_clist_remove (clist, row);
	}

	gtk_object_unref (GTK_OBJECT (client));
}

void
on_dialog_preferences_button_ok_clicked (GtkButton *button)
{
	gtk_widget_destroy (glade_xml_get_widget (gtk_object_get_data (GTK_OBJECT (button), "xml"), "dialog_preferences"));
}

void
on_radiobutton_debug_level_toggled (GtkToggleButton* toggle_button)
{
	g_warning ("Not yet implemented!");
}

/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/

/**
 * preferences:
 *
 * Pops up the preferences dialog.
 **/
void
preferences ()
{
	GConfClient*		client = gconf_client_get_default ();
	GladeXML*		xml;
	GtkCList*		clist;
	GtkWidget*		widget;
	Bonobo_Unknown		bag;
	CORBA_Environment	ev;
	gint			i;

	g_return_if_fail (xml = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "dialog_preferences"));
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "dialog_preferences_button_ok")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "button_camera_delete")), "xml", xml);
	glade_xml_signal_autoconnect (xml);
	g_return_if_fail (clist = GTK_CLIST (glade_xml_get_widget (xml, "clist_cameras")));

	/* Init exception. */
	CORBA_exception_init (&ev);

	/* Get the current setting for prefix. */
	gtk_widget_show (widget = bonobo_widget_new_control ("config:/apps/" PACKAGE "/prefix", NULL));
	bonobo_control_frame_control_activate (bonobo_widget_get_control_frame (BONOBO_WIDGET (widget)));
	gtk_container_add (GTK_CONTAINER (glade_xml_get_widget (xml, "vbox_prefix")), widget);

	/* Get the current setting for debug level. */
	gtk_widget_show (widget = bonobo_widget_new_control ("config:/apps/" PACKAGE "/debug_level", NULL));
	bonobo_control_frame_control_activate (bonobo_widget_get_control_frame (BONOBO_WIDGET (widget)));
	gtk_container_add (GTK_CONTAINER (glade_xml_get_widget (xml, "vbox_debug_level")), widget);

	/* Get the current settings for cameras. */
	for (i = 0; ; i++) {
		gchar* tmp = g_strdup_printf ("/apps/" PACKAGE "/camera/%i", i);
		if (gconf_client_dir_exists (client, tmp, NULL)) {
			gchar* moniker = g_strconcat ("config:", tmp, NULL);
			bag = bonobo_get_object (moniker, "IDL:Bonobo/PropertyBag:1.0", &ev);
			g_free (moniker);
			if (BONOBO_EX (&ev)) g_warning (_("Could not get property bag of '%s'!\n(%s)"), tmp, bonobo_exception_get_text (&ev));
		if (bag != CORBA_OBJECT_NIL) {
			Bonobo_Unknown	property;
			CORBA_any*	value_name;
			CORBA_any*	value_model;
			CORBA_any*	value_port;
			gchar*		row[3];
			
			/* Name */
			property = Bonobo_PropertyBag_getPropertyByName (bag, "name", &ev);
			if (BONOBO_EX (&ev)) {
				g_warning (_("Could not get property 'name' of '%s'!\n(%s)"), tmp, bonobo_exception_get_text (&ev));
				continue;
			}
			value_name = Bonobo_Property_getValue (property, &ev);
			if (BONOBO_EX (&ev)) {
				g_warning (_("Could not get value of property 'name' of '%s'!\n(%s)"), tmp, bonobo_exception_get_text (&ev));
				continue;
			}
			if (!bonobo_arg_type_is_equal(value_name->_type, BONOBO_ARG_STRING, NULL)) {
				g_warning (_("Property 'name' of '%s' is not of type 'string'!\n(%s)"), tmp, bonobo_exception_get_text (&ev));
				continue;
			}

			/* Port */
			property = Bonobo_PropertyBag_getPropertyByName (bag, "port", &ev);
			if (BONOBO_EX (&ev)) {
				g_warning (_("Could not get property 'port' of '%s'!\n(%s)"), tmp, bonobo_exception_get_text (&ev));
				continue;
			}
			value_port = Bonobo_Property_getValue (property, &ev);
			if (BONOBO_EX (&ev)) {
				g_warning (_("Could not get value of property 'port' of '%s'!\n(%s)"), tmp, bonobo_exception_get_text (&ev));
				continue;
			}
			if (!bonobo_arg_type_is_equal(value_port->_type, BONOBO_ARG_STRING, NULL)) {
				g_warning (_("Property 'port' of '%s' is not of type 'string'!\n(%s)"), tmp, bonobo_exception_get_text (&ev));
				continue;
			}

			/* Model */
			property = Bonobo_PropertyBag_getPropertyByName (bag, "model", &ev);
			if (BONOBO_EX (&ev)) {
				g_warning (_("Could not get property 'model' of '%s'!\n(%s)"), tmp, bonobo_exception_get_text (&ev));
				continue;
			}
			value_model = Bonobo_Property_getValue (property, &ev);
			if (BONOBO_EX (&ev)) {
				g_warning (_("Could not get value of property 'model' of '%s'!\n(%s)"), tmp, bonobo_exception_get_text (&ev));
				continue;
			}
			if (!bonobo_arg_type_is_equal(value_model->_type, BONOBO_ARG_STRING, NULL)) {
				g_warning (_("Property 'model' of '%s' is not of type 'string'!\n(%s)"), tmp, bonobo_exception_get_text (&ev));
				continue;
			}
			
				row[0] = BONOBO_ARG_GET_STRING (value_name);
				row[1] = BONOBO_ARG_GET_STRING (value_model);
				row[2] = BONOBO_ARG_GET_STRING (value_port);
				gtk_clist_set_row_data (clist, gtk_clist_append (clist, row), GINT_TO_POINTER (i));
			}
		} else {
			g_free (tmp);
			break;
		}
		g_free (tmp);
	}

	/* Free exception. */
	CORBA_exception_free (&ev);

	gtk_object_unref (GTK_OBJECT (client));
}


