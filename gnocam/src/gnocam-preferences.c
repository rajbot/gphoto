#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gphoto2.h>

#include "gnocam-preferences.h"

#include <gal/util/e-util.h>
#include <gconf/gconf-client.h>
#include <glade/glade.h>
#include <bonobo.h>

#define PARENT_TYPE GNOME_TYPE_DIALOG
static GnomeDialogClass* parent_class = NULL;

struct _GnoCamPreferencesPrivate {
	GConfClient*	client;
	guint		cnxn;

	GtkWidget*	clist;
};


/**************/
/* Prototypes */
/**************/

void on_druid_cancel 				(GnomeDruid* druid);
void on_druidpagestandard_model_prepare 	(GnomeDruidPage* page, GtkWidget* druid);
void on_druidpagestandard_port_prepare 		(GnomeDruidPage* page, GtkWidget* druid);
void on_druid_finish 				(GnomeDruidPage* page, GtkWidget* druid);

/************************/
/* GConf - notification */
/************************/

static void
on_cameras_changed (GConfClient *client, guint cnxn_id, GConfEntry* entry, gpointer user_data)
{
	GnoCamPreferences*	preferences;
	gint			i, row;
	GSList*			list;
	GtkCList*		clist;

	g_return_if_fail (GNOCAM_IS_PREFERENCES (user_data));
	preferences = GNOCAM_PREFERENCES (user_data);
	clist = GTK_CLIST (preferences->priv->clist);
	list = gconf_client_get_list (preferences->priv->client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, NULL);

	/* Delete deleted cameras */
	for (row = 0; row < clist->rows; ) {
		gchar*	name = NULL;

		gtk_clist_get_text (clist, row, 0, &name);
		for (i = 0; i < g_slist_length (list); i++) {
			if (!strcmp (g_slist_nth_data (list, i), name)) break;
		}
		if (i == g_slist_length (list)) gtk_clist_remove (clist, row);
		else row++;
	}

	/* Append the new cameras */
	for (i = 0; i < g_slist_length (list); i += 3) {
		for (row = 0; row < clist->rows; row++) {
			gchar* name = NULL;
			
			gtk_clist_get_text (clist, row, 0, &name);
			if (!strcmp (g_slist_nth_data (list, i), name)) break;
		}
		if (row == clist->rows) {
			gchar*  row [3];

        	        row [0] = g_slist_nth_data (list, i);
	                row [1] = g_slist_nth_data (list, i + 1);
	                row [2] = g_slist_nth_data (list, i + 2);
	                gtk_clist_append (GTK_CLIST (clist), row);
		}
	}

	/* Free the list */
	for (i = 0; i < g_slist_length (list); i++) g_free (g_slist_nth_data (list, i));
	g_slist_free (list);
}

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
        } else g_warning (_("Could not get number of supported models!\n(%s)"), gp_result_as_string (number_of_models));
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
				g_warning (_("Could not get information about port number %i!\n(%s)"), i, gp_result_as_string (result));
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
        } else g_warning (_("Could not get abilities for model '%s'!\n(%s)"), gtk_entry_get_text (entry_model), gp_result_as_string (result));
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
	GSList*		list;

	g_return_if_fail (xml = gtk_object_get_data (GTK_OBJECT (druid), "xml"));
	g_return_if_fail (model = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "entry_model"))));
	g_return_if_fail (name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "entry_name"))));
	g_return_if_fail (port = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "entry_port"))));
	g_return_if_fail (client = gconf_client_get_default ());

	/* Get the list */
	list = gconf_client_get_list (client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, NULL);

	/* Append the new entries */
	list = g_slist_append (list, g_strdup (name));
	list = g_slist_append (list, g_strdup (model));
	list = g_slist_append (list, g_strdup (port));

	/* Tell gconf about the new list */
	gconf_client_set_list (client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, list, NULL);

	/* Free the list */
	for (i = 0; i < g_slist_length (list); i++) g_free (g_slist_nth_data (list, i));
	g_slist_free (list);

	gtk_widget_unref (glade_xml_get_widget (xml, "window_druid"));
	gtk_object_unref (GTK_OBJECT (client));
}

void
on_druid_cancel (GnomeDruid* druid)
{
	gtk_widget_destroy (glade_xml_get_widget (gtk_object_get_data (GTK_OBJECT (druid), "xml"), "window_druid"));
}

static void
on_new_clicked (GtkButton* button)
{
	GladeXML*	xml;

	/* Create the druid. */
	g_return_if_fail (xml = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "window_druid"));
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "druid")), "xml", xml);
	glade_xml_signal_autoconnect (xml);
}

static void
on_delete_clicked (GtkButton *button, gpointer user_data)
{
	GnoCamPreferences*	preferences;
	GList*			selection;
	GtkCList*		clist;

	preferences = GNOCAM_PREFERENCES (user_data);
	clist = GTK_CLIST (preferences->priv->clist);

	/* Remove the rows in the camera list. */
	while ((selection = g_list_first (clist->selection))) {
		gint	row;
		GSList*	list;
		gint	i;

		row = GPOINTER_TO_INT (selection->data);
		list = gconf_client_get_list (preferences->priv->client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, NULL);

		for (i = 0; i < 3; i++) {
			GSList*	link;

			link = g_slist_nth (list, row);
			list = g_slist_remove_link (list, link);
			g_free (g_slist_nth_data (link, 1));
			g_slist_free (link);
		}

		/* Tell GConf about the new list */
		gconf_client_set_list (preferences->priv->client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, list, NULL);

		/* Free the list */
		for (i = 0; i < g_slist_length (list); i++) g_free (g_slist_nth_data (list, i));
		g_slist_free (list);
		
		gtk_clist_remove (clist, row);
	}
}

/*************************/
/* Gnome-Dialog specific */
/*************************/

static void
gnocam_preferences_destroy (GtkObject* object)
{
	GnoCamPreferences*	preferences;

	preferences = GNOCAM_PREFERENCES (object);

	gconf_client_notify_remove (preferences->priv->client, preferences->priv->cnxn);
	gtk_object_unref (GTK_OBJECT (preferences->priv->client));
	g_free (preferences->priv);

	(*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
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
gnocam_preferences_new (GtkWindow* parent)
{
	GnoCamPreferences*	new;
	GtkWidget*		label;
	GtkWidget*		vbuttonbox;
	GtkWidget*		hbox;
	GtkWidget*		vbox;
	GtkWidget*		widget;
	GtkWidget*		frame;
	GtkWidget*		notebook;
	GtkWidget*		button;
	const gchar*		buttons [] = {GNOME_STOCK_BUTTON_OK, NULL};
	gchar*			titles [] = {_("Name"), _("Model"), _("Port"), NULL};

	new = gtk_type_new (GNOCAM_TYPE_PREFERENCES);
	gnome_dialog_constructv (GNOME_DIALOG (new), _(PACKAGE " - Preferences"), buttons);
	gnome_dialog_set_close (GNOME_DIALOG (new), TRUE);
	new->priv->client = gconf_client_get_default ();
	gconf_client_add_dir (new->priv->client, "/apps" PACKAGE, GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);
	gnome_dialog_set_parent (GNOME_DIALOG (new), parent);

	/* Create the notebook */
	gtk_widget_show (notebook = gtk_notebook_new ());
	gtk_container_add (GTK_CONTAINER (GNOME_DIALOG (new)->vbox), notebook);

	/* Create the page for GNOME settings */
	gtk_widget_show (vbox = gtk_vbox_new (FALSE, 0));
	gtk_widget_show (label = gtk_label_new (_("GNOME")));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
	gtk_widget_show (frame = gtk_frame_new (_("Cameras")));
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_container_add (GTK_CONTAINER (vbox), frame);
	gtk_widget_show (hbox = gtk_hbox_new (FALSE, 10));
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
	gtk_container_add (GTK_CONTAINER (frame), hbox);
	gtk_widget_show (new->priv->clist = gtk_clist_new_with_titles (3, titles));
	new->priv->cnxn = gconf_client_notify_add (new->priv->client, "/apps/" PACKAGE "/cameras", on_cameras_changed, new, NULL, NULL);
	on_cameras_changed (new->priv->client, 0, NULL, new);
	gtk_container_add (GTK_CONTAINER (hbox), new->priv->clist);
	gtk_widget_show (vbuttonbox = gtk_vbutton_box_new ());
	gtk_container_add (GTK_CONTAINER (hbox), vbuttonbox);
	gtk_widget_show (button = gtk_button_new_with_label (_("New")));
	gtk_container_add (GTK_CONTAINER (vbuttonbox), button);
	gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (on_new_clicked), new);
	gtk_widget_show (button = gtk_button_new_with_label (_("Delete")));
	gtk_container_add (GTK_CONTAINER (vbuttonbox), button);
	gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (on_delete_clicked), new);

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
	gtk_widget_show (widget = bonobo_widget_new_control ("config:/apps/" PACKAGE "/prefix", NULL));
	bonobo_control_frame_control_activate (bonobo_widget_get_control_frame (BONOBO_WIDGET (widget)));
	gtk_container_add (GTK_CONTAINER (vbox), widget);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_preferences, "GnoCamPreferences", GnoCamPreferences, gnocam_preferences_class_init, gnocam_preferences_init, PARENT_TYPE)

