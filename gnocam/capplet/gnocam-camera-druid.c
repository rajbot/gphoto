
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gphoto2.h>

#include <gal/util/e-util.h>
#include <gconf/gconf-client.h>

#include "gnocam-camera-druid.h"

#define PARENT_TYPE GTK_TYPE_WINDOW
static GtkWindowClass* parent_class = NULL;

struct _GnoCamCameraDruidPrivate {

	GConfClient*	client;

	GnomeDruid*		druid;
	GnomeDruidPageFinish*	page_finish;
	GtkCombo*		combo_model;
	GtkCombo*		combo_port;
	GtkEntry*		entry_name;
};

#define PAGE_START 						\
"Hello and Welcome to GnoCam!\n"				\
"\n"								\
"With GnoCam, you can access your digital camera through\n"	\
"various Gnome applications. However, you first have to\n"	\
"tell us what camera model you have got."

#define PAGE_FINISH 						\
"GnoCam now has all information it needs to access your\n"	\
"camera.\n"							\
"\n"								\
"Thank you for using GnoCam!"

/*************/
/* Callbacks */
/*************/

static void
on_model_changed (GtkEditable* editable, gpointer user_data)
{
	GnoCamCameraDruid*      druid;
	gchar*			model;

	druid = GNOCAM_CAMERA_DRUID (user_data);

	model = gtk_entry_get_text (GTK_ENTRY (editable));
	if (strcmp ("", model)) gnome_druid_set_buttons_sensitive (druid->priv->druid, TRUE, TRUE, TRUE);
	else gnome_druid_set_buttons_sensitive (druid->priv->druid, TRUE, FALSE, TRUE);
}

static void
on_name_changed (GtkEditable* editable, gpointer user_data)
{
	GnoCamCameraDruid*      druid;
	gchar*			name;

	druid = GNOCAM_CAMERA_DRUID (user_data);

	name = gtk_entry_get_text (GTK_ENTRY (editable));
	if (strcmp ("", name)) gnome_druid_set_buttons_sensitive (druid->priv->druid, TRUE, TRUE, TRUE);
	else gnome_druid_set_buttons_sensitive (druid->priv->druid, TRUE, FALSE, TRUE);
}

static void
on_page_model_prepare (GnomeDruidPage* page, GtkWidget* d, gpointer user_data)
{
	GnoCamCameraDruid*	druid;
        gchar           	buffer[1024];
        gchar*          	model;
        gint            	number_of_models, i;
        GList*          	list = NULL;

	g_return_if_fail (user_data);
	druid = GNOCAM_CAMERA_DRUID (user_data);

        /* Build model list. */
        if ((number_of_models = gp_camera_count ()) >= 0) {
                for (i = 0; i < number_of_models; i++) {
                        if (gp_camera_name (i, buffer) != GP_OK) strcpy (buffer, "?");
                        list = g_list_append (list, g_strdup (buffer));
                }
                if (!list) list = g_list_append (NULL, g_strdup (""));
                model = g_strdup (gtk_entry_get_text (GTK_ENTRY (druid->priv->combo_model->entry)));
                gtk_combo_set_popdown_strings (druid->priv->combo_model, list);
                gtk_entry_set_text (GTK_ENTRY (druid->priv->combo_model->entry), model);
        } else g_warning (_("Could not get number of supported models!\n(%s)"), gp_result_as_string (number_of_models));
}

static void
on_page_port_prepare (GnomeDruidPage* page, GtkWidget* d, gpointer user_data)
{
	GnoCamCameraDruid*	druid;
        GList*          	list = NULL;
        gint            	i, result;
        CameraPortInfo  	info;
        CameraAbilities 	abilities;
        gchar*          	port;

	g_return_if_fail (user_data);

	druid = GNOCAM_CAMERA_DRUID (user_data);
        
        if ((result = gp_camera_abilities_by_name (gtk_entry_get_text (GTK_ENTRY (druid->priv->combo_model->entry)), &abilities)) == GP_OK) {
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
                if (!list) gnome_druid_set_page (druid->priv->druid, GNOME_DRUID_PAGE (druid->priv->page_finish));
                port = g_strdup (gtk_entry_get_text (GTK_ENTRY (druid->priv->combo_port->entry)));
                gtk_combo_set_popdown_strings (druid->priv->combo_port, list);
		gtk_entry_set_text (GTK_ENTRY (druid->priv->combo_port->entry), port);
        } else 
		g_warning (_("Could not get abilities for model '%s'!\n(%s)"), 
			gtk_entry_get_text (GTK_ENTRY (druid->priv->combo_model->entry)), gp_result_as_string (result));
}

static void
on_finish (GnomeDruidPage* page, GtkWidget* d, gpointer user_data)
{
	GnoCamCameraDruid*	druid;
        gchar*          	model;
        gchar*          	name;
        gchar*          	port;
        gint            	i;
        GSList*         	list;

	g_return_if_fail (user_data);

	druid = GNOCAM_CAMERA_DRUID (user_data);

        model = gtk_entry_get_text (GTK_ENTRY (druid->priv->combo_model->entry));
        name = gtk_entry_get_text (GTK_ENTRY (druid->priv->entry_name));
        port = gtk_entry_get_text (GTK_ENTRY (druid->priv->combo_port->entry));

        /* Get the list */
        list = gconf_client_get_list (druid->priv->client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, NULL);

        /* Append the new entries */
        list = g_slist_append (list, g_strdup (name));
        list = g_slist_append (list, g_strdup (model));
        list = g_slist_append (list, g_strdup (port));

        /* Tell gconf about the new list */
        gconf_client_set_list (druid->priv->client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, list, NULL);

        /* Free the list */
        for (i = 0; i < g_slist_length (list); i++) g_free (g_slist_nth_data (list, i));
        g_slist_free (list);

	gtk_widget_unref (GTK_WIDGET (druid));
}

static void
on_cancel (GnomeDruid* d, gpointer user_data)
{
	GnoCamCameraDruid*	druid;

	druid = GNOCAM_CAMERA_DRUID (user_data);
        gtk_widget_unref (GTK_WIDGET (druid));
}

/*************/
/* Gtk stuff */
/*************/

static void
gnocam_camera_druid_destroy (GtkObject* object)
{
	GnoCamCameraDruid*	druid;

	druid = GNOCAM_CAMERA_DRUID (object);

	gtk_object_unref (GTK_OBJECT (druid->priv->client));

	g_free (druid->priv);
	druid->priv = NULL;

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_camera_druid_class_init (GnoCamCameraDruidClass* klass)
{
	GtkObjectClass*         object_class;

	object_class = GTK_OBJECT_CLASS (klass);
        object_class->destroy = gnocam_camera_druid_destroy;

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_camera_druid_init (GnoCamCameraDruid* druid)
{
	druid->priv = g_new0 (GnoCamCameraDruidPrivate, 1);
}

GtkWidget*
gnocam_camera_druid_new (GConfClient* client, GtkWindow* window)
{
	GdkColor		blue;
	GdkColor		grey;
	GnoCamCameraDruid*	new;
	GtkWidget*		page;
	GtkWidget*		frame;
	GtkWidget*		label;
	GtkWidget*		hbox;

	new = gtk_type_new (GNOCAM_TYPE_CAMERA_DRUID);
	if (window) gtk_window_set_transient_for (GTK_WINDOW (new), window);
	gtk_object_ref (GTK_OBJECT (new->priv->client = client));

	blue.red = 24 * 256;
	blue.green = 24 * 256;
	blue.blue = 112 * 256;

	grey.red = 208 * 256;
	grey.green = 212 * 256;
	grey.blue = 208 * 256;

	/* Druid */
	new->priv->druid = GNOME_DRUID (gnome_druid_new ());
	gtk_widget_show (GTK_WIDGET (new->priv->druid));
	gtk_container_add (GTK_CONTAINER (new), GTK_WIDGET (new->priv->druid));
	gtk_signal_connect (GTK_OBJECT (new->priv->druid), "cancel", GTK_SIGNAL_FUNC (on_cancel), new);
	
	/* Start page */
	page = gnome_druid_page_start_new ();
	gtk_widget_show (page);
	gnome_druid_page_start_set_text (GNOME_DRUID_PAGE_START (page), PAGE_START);
	gnome_druid_page_start_set_title (GNOME_DRUID_PAGE_START (page), _("Welcome to " PACKAGE "!"));
	gnome_druid_page_start_set_logo (GNOME_DRUID_PAGE_START (page), gdk_imlib_load_image (IMAGEDIR "/gnocam.png"));
	gdk_color_alloc (gtk_widget_get_colormap (page), &blue);
	gnome_druid_page_start_set_logo_bg_color (GNOME_DRUID_PAGE_START (page), &blue);
	gdk_color_alloc (gtk_widget_get_colormap (page), &grey);
	gnome_druid_page_start_set_textbox_color (GNOME_DRUID_PAGE_START (page), &grey);
	gnome_druid_append_page (GNOME_DRUID (new->priv->druid), GNOME_DRUID_PAGE (page));

	/* First page */
        page = gnome_druid_page_standard_new ();
	gnome_druid_page_standard_set_logo (GNOME_DRUID_PAGE_STANDARD (page), gdk_imlib_load_image (IMAGEDIR "/gnocam.png"));
	gdk_color_alloc (gtk_widget_get_colormap (page), &blue);
	gnome_druid_page_standard_set_logo_bg_color (GNOME_DRUID_PAGE_STANDARD (page), &blue);
	gtk_container_set_border_width (GTK_CONTAINER (GNOME_DRUID_PAGE_STANDARD (page)->vbox), 10);
	gnome_druid_page_standard_set_title (GNOME_DRUID_PAGE_STANDARD (page), _("Model?"));
	gtk_signal_connect (GTK_OBJECT (page), "prepare", GTK_SIGNAL_FUNC (on_page_model_prepare), new);
	gtk_widget_show (page);
	gnome_druid_append_page (GNOME_DRUID (new->priv->druid), GNOME_DRUID_PAGE (page));

	/* Model */
	label = gtk_label_new (_("Please choose a camera model. If you can't find yours, you can still try a similar model."));
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page)->vbox), label, FALSE, FALSE, 10);
	frame = gtk_frame_new (_("Model"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page)->vbox), frame, FALSE, FALSE, 10);
	hbox = gtk_hbox_new (FALSE, 10);
	gtk_widget_show (hbox);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
	gtk_container_add (GTK_CONTAINER (frame), hbox);
	label = gtk_label_new (_("Model:"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 10);
	new->priv->combo_model = GTK_COMBO (gtk_combo_new ());
	gtk_widget_show (GTK_WIDGET (new->priv->combo_model));
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (new->priv->combo_model), FALSE, FALSE, 10);
	gtk_signal_connect (GTK_OBJECT (GTK_COMBO (new->priv->combo_model)->entry), "changed", GTK_SIGNAL_FUNC (on_model_changed), new);

	/* Name */
	label = gtk_label_new (_("Your camera will be identified by a name. You probably want to use a short, easy to remember name for your camera, for "
		"example \"camera\"."));
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page)->vbox), label, FALSE, FALSE, 10);
	frame = gtk_frame_new (_("Name"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page)->vbox), frame, FALSE, FALSE, 10);
	hbox = gtk_hbox_new (FALSE, 10);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
	gtk_widget_show (hbox);
	gtk_container_add (GTK_CONTAINER (frame), hbox);
	label = gtk_label_new (_("Name:"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 10);
	new->priv->entry_name = GTK_ENTRY (gtk_entry_new ());
	gtk_entry_set_text (new->priv->entry_name, _("camera"));
	gtk_widget_show (GTK_WIDGET (new->priv->entry_name));
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (new->priv->entry_name), FALSE, FALSE, 10);
	gtk_signal_connect (GTK_OBJECT (new->priv->entry_name), "changed", GTK_SIGNAL_FUNC (on_name_changed), new);

	/* Second page */
	page = gnome_druid_page_standard_new ();
	gtk_container_set_border_width (GTK_CONTAINER (GNOME_DRUID_PAGE_STANDARD (page)->vbox), 10);
	gtk_signal_connect (GTK_OBJECT (page), "prepare", GTK_SIGNAL_FUNC (on_page_port_prepare), new);
	gnome_druid_page_standard_set_title (GNOME_DRUID_PAGE_STANDARD (page), _("Port?"));
	gnome_druid_page_standard_set_logo (GNOME_DRUID_PAGE_STANDARD (page), gdk_imlib_load_image (IMAGEDIR "/gnocam.png"));
	gdk_color_alloc (gtk_widget_get_colormap (page), &blue);
	gnome_druid_page_standard_set_logo_bg_color (GNOME_DRUID_PAGE_STANDARD (page), &blue);
	gtk_widget_show (page);
	gnome_druid_append_page (new->priv->druid, GNOME_DRUID_PAGE (page));

	/* Port */
	label = gtk_label_new (_("How did you connect your camera to this computer?"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page)->vbox), label, FALSE, FALSE, 10);
	frame = gtk_frame_new (_("Port"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page)->vbox), frame, FALSE, FALSE, 10);
	hbox = gtk_hbox_new (FALSE, 10);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
	gtk_widget_show (hbox);
	gtk_container_add (GTK_CONTAINER (frame), hbox);
	label = gtk_label_new (_("Port:"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 10);
	new->priv->combo_port = GTK_COMBO (gtk_combo_new ());
	gtk_widget_show (GTK_WIDGET (new->priv->combo_port));
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (new->priv->combo_port), FALSE, FALSE, 10);

	/* Finish page */
	new->priv->page_finish = GNOME_DRUID_PAGE_FINISH (gnome_druid_page_finish_new ());
	gtk_signal_connect (GTK_OBJECT (new->priv->page_finish), "finish", GTK_SIGNAL_FUNC (on_finish), new);
	gtk_widget_show (GTK_WIDGET (new->priv->page_finish));
	gnome_druid_page_finish_set_text (new->priv->page_finish, PAGE_FINISH);
	gnome_druid_page_finish_set_title (new->priv->page_finish, _("Thank you!"));
	gnome_druid_append_page (new->priv->druid, GNOME_DRUID_PAGE (new->priv->page_finish));
	gnome_druid_page_finish_set_logo (new->priv->page_finish, gdk_imlib_load_image (IMAGEDIR "/gnocam.png"));
	gdk_color_alloc (gtk_widget_get_colormap (GTK_WIDGET (new->priv->page_finish)), &blue);
	gnome_druid_page_finish_set_logo_bg_color (new->priv->page_finish, &blue);
	gdk_color_alloc (gtk_widget_get_colormap (GTK_WIDGET (new->priv->page_finish)), &grey);
	gnome_druid_page_finish_set_textbox_color (new->priv->page_finish, &grey);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_camera_druid, "GnoCamCameraDruid", GnoCamCameraDruid, gnocam_camera_druid_class_init, gnocam_camera_druid_init, PARENT_TYPE)


