#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gphoto2.h>
#include <bonobo.h>

#include "utils.h"
#include "e-shell-constants.h"

/********************/
/* Type Definitions */
/********************/

typedef struct {
	Camera*		camera;
	CameraWidget*	widget;
	gchar*		folder;
	gchar*		file;
} WidgetData;

/**************/
/* Prototypes */
/**************/

static void on_button_clicked 		(BonoboUIComponent *uic, gpointer user_data, const char *cname);
static void on_togglebutton_clicked	(BonoboUIComponent* uic, const char* path, Bonobo_UIComponent_EventType type, const char* state, gpointer user_data);

/********************/
/* Helper functions */
/********************/

static void
set_config (WidgetData* data)
{
	gint	result;

        if (data->file) result = gp_camera_file_config_set (data->camera, gp_widget_root (data->widget), data->folder, data->file);
        else if (data->folder) result = gp_camera_folder_config_set (data->camera, gp_widget_root (data->widget), data->folder);
        else result = gp_camera_config_set (data->camera, gp_widget_root (data->widget));
        if (result != GP_OK)
                g_warning (_("Could not set configuration of '%s'!\n(%s)"), gp_widget_label (data->widget), gp_camera_result_as_string (data->camera, result));
}

/*************/
/* Callbacks */
/*************/

static void
on_button_clicked (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	WidgetData*		data = (WidgetData*) user_data;
	CameraWidgetCallback	callback;
	gint			result;

	g_return_if_fail (data);

	callback = gp_widget_callback (data->widget);
	if ((result = callback (data->camera, data->widget)) != GP_OK) 
		g_warning (_("Could not execute command '%s'! (%s)"), gp_widget_label (data->widget), gp_camera_result_as_string (data->camera, result));
}

static void
on_entry_changed (GtkObject* object, gpointer user_data)
{
	WidgetData*	data = (WidgetData*) user_data;
	gchar*		value_string = NULL;
	gchar*		value_string_new = NULL;
	gint		result;

	g_return_if_fail (data);

	if ((result = gp_widget_value_get (data->widget, &value_string)) != GP_OK)
		g_warning (_("Could not get value of widget '%s': %s"), gp_widget_label (data->widget), gp_result_as_string (result));

	value_string_new = gtk_entry_get_text (GTK_ENTRY (object));
	if (!value_string || strcmp (value_string, value_string_new)) {
	
		if ((result = gp_widget_value_set (data->widget, value_string_new)) != GP_OK)
			g_warning (_("Could not set value of widget '%s': %s"), gp_widget_label (data->widget), gp_result_as_string (result));
		else set_config (data);
	}
}

static void
on_adjustment_value_changed (GtkObject* object, gpointer user_data)
{
	WidgetData*	data = (WidgetData*) user_data;
	gfloat		f, f_new;
	gint		result;

	g_return_if_fail (data);

	if ((result = gp_widget_value_get (data->widget, &f)) != GP_OK)
		g_warning (_("Could not get value of widget '%s': %s"), gp_widget_label (data->widget), gp_result_as_string (result));

	f_new = GTK_ADJUSTMENT (object)->value;
	if (f != f_new) {
		if ((result = gp_widget_value_set (data->widget, &f_new)) != GP_OK)
			g_warning (_("Could not set value of widget '%s': %s"), gp_widget_label (data->widget), gp_result_as_string (result));
		else set_config (data);
	}
}

static void
on_togglebutton_clicked (BonoboUIComponent* component, const char* path, Bonobo_UIComponent_EventType t, const char* state, gpointer user_data)
{
	WidgetData*     	data = (WidgetData*) user_data;
	gint			result;
	gint			value_int;
	gchar*			value_string = NULL;
	gchar*			value_string_new = NULL;
	CameraWidgetType	type;
	
	g_return_if_fail (data);
	type = gp_widget_type (data->widget);
	g_return_if_fail ((type == GP_WIDGET_MENU) || (type == GP_WIDGET_RADIO) || (type == GP_WIDGET_TOGGLE));

	/* We are not interested in untoggled radio buttons */
	if ((type != GP_WIDGET_TOGGLE) && !strcmp (state, "0")) return;

	switch (type) {
	case GP_WIDGET_TOGGLE:
		value_int = atoi (state);
		if ((result = gp_widget_value_set (data->widget, &value_int)) != GP_OK)
			g_warning (_("Could not set value of widget '%s': %s"), gp_widget_label (data->widget), gp_result_as_string (result));
		else set_config (data);
		break;
	case GP_WIDGET_MENU:
	case GP_WIDGET_RADIO:
		for (; *path != 0; path++) if (*path == '_') break;
		path++;

		/* Old value */
		if ((result = gp_widget_value_get (data->widget, &value_string)) != GP_OK)
			g_warning (_("Could not get value of widget '%s': %s"), gp_widget_label (data->widget), gp_result_as_string (result));

		/* Set new value only if different from old one */
		value_string_new = gp_widget_choice (data->widget, atoi (path));
		if (!value_string || strcmp (value_string, value_string_new)) {
			if ((result = gp_widget_value_set (data->widget, value_string_new)) != GP_OK)
				g_warning (_("Could not set value of widget '%s': %s"), gp_widget_label (data->widget), gp_result_as_string (result));
			else set_config (data);
		}
		break;
	default:
		g_assert_not_reached ();
	}
}

static void
on_date_changed (GtkObject* object, gpointer user_data)
{
	WidgetData*	data = (WidgetData*) user_data;
	gint		i, i_new;
	
	g_return_if_fail (data);

	g_return_if_fail (gp_widget_value_get (data->widget, &i) == GP_OK);
	i_new = (int) gnome_date_edit_get_date (GNOME_DATE_EDIT (object));
	if (i != i_new) {
		g_return_if_fail (gp_widget_value_set (data->widget, &i_new) == GP_OK);
		set_config (data);
	}
}

/*************/
/* Functions */
/*************/

#define PLACEHOLDER	"<placeholder name=\"Configuration\"><submenu name=\"Configuration\" _label=\"Configuration\"/></placeholder>"

#define SUBMENU			"<submenu name=\"%i\" _tip=\"%s\" _label=\"%s\"/>"
#define MENUITEM		"<menuitem name=\"%i\" verb=\"\"/>"
#define MENUITEM2		"<menuitem name=\"%i_%i\" verb=\"\"/>"
#define MENUITEM_CONTROL	"<control name=\"%i\" _tip=\"%s\"/>"

#define CMD_BUTTON 		"<cmd name=\"%i\" _label=\"%s\" _tip=\"%s\"/>"
#define CMD_TOGGLE 		"<cmd name=\"%i\" _label=\"%s\" _tip=\"%s\" type=\"toggle\" state=\"%i\"/>"
#define CMD_RADIO		"<cmd name=\"%i_%i\" _label=\"%s\" _tip=\"%s\" type=\"radio\" group=\"%s\"/>"

static void
setup_menuitems (BonoboUIComponent* component, gchar* path, CameraWidget* widget)
{
	gchar*			tmp;
	CameraWidgetType	type;

	type = gp_widget_type (widget);
        switch (type) {
        case GP_WIDGET_WINDOW:
                break;
        case GP_WIDGET_RADIO:
        case GP_WIDGET_MENU:
        case GP_WIDGET_SECTION:
                tmp = g_strdup_printf (SUBMENU, gp_widget_id (widget), gp_widget_label (widget), gp_widget_label (widget));
                bonobo_ui_component_set_translate (component, path, tmp, NULL);
                g_free (tmp);

		if (type != GP_WIDGET_SECTION) {
			gint	j;
			gchar*	extended_path;

			extended_path = g_strdup_printf ("%s/%i", path, gp_widget_id (widget));
			for (j = 0; j < gp_widget_choice_count (widget); j++) {
				tmp = g_strdup_printf (MENUITEM2, gp_widget_id (widget), j);
        	                bonobo_ui_component_set_translate (component, extended_path, tmp, NULL);
                	        g_free (tmp);
			}
			g_free (extended_path);
		}
                break;
        case GP_WIDGET_TEXT:
        case GP_WIDGET_DATE:
        case GP_WIDGET_RANGE:
                tmp = g_strdup_printf (MENUITEM_CONTROL, gp_widget_id (widget), gp_widget_label (widget));
                bonobo_ui_component_set_translate (component, path, tmp, NULL);
                g_free (tmp);
                break;
        default:
                tmp = g_strdup_printf (MENUITEM, gp_widget_id (widget));
                bonobo_ui_component_set_translate (component, path, tmp, NULL);
                g_free (tmp);
		break;
        }

}

static void
setup_control (BonoboUIComponent* component, gchar* path, CameraWidget* widget, Camera* camera, gchar* folder, gchar* file)
{
	gfloat			min = 0.0;
	gfloat 			max = 0.0;
	gfloat			increment = 0.0;
	gchar*			tmp;
	gchar*			value_string = NULL;
	gfloat			value_float = 0.0;
	gint			value_int = 0;
	gint			result;
	GtkWidget*		hbox;
	GtkWidget*		label;
	GtkWidget*		hscale;
	GtkWidget*		date_edit;
	GtkWidget*		entry;
	GtkObject*		adjustment;
	BonoboControl*		control;
	WidgetData*		data;
	CameraWidgetType	type;
	
	type = gp_widget_type (widget);
	g_return_if_fail ((type == GP_WIDGET_RANGE) || (type == GP_WIDGET_DATE) || (type == GP_WIDGET_TEXT));

	/* Data we need */
	data = g_new (WidgetData, 1);
	data->camera = camera;
	data->widget = widget;
	data->folder = g_strdup (folder);
	data->file = g_strdup (file);

	/* Basic layout */
	hbox = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hbox);
	label = gtk_label_new (gp_widget_label (widget));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	switch (type) {
        case GP_WIDGET_RANGE:
                if ((result = gp_widget_value_get (widget, &value_float)) != GP_OK) 
			g_warning (_("Could not get value for widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
                if ((result = gp_widget_range_get (widget, &min, &max, &increment)) != GP_OK) 
			g_warning (_("Could not get range of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));

                /* Create the widget */
                adjustment = gtk_adjustment_new (value_float, min, max, increment, 0, 0);
                gtk_signal_connect (adjustment, "value_changed", GTK_SIGNAL_FUNC (on_adjustment_value_changed), data);
                hscale = gtk_hscale_new (GTK_ADJUSTMENT (adjustment));
		gtk_widget_show (hscale);
                gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DISCONTINUOUS);
                gtk_box_pack_end (GTK_BOX (hbox), hscale, TRUE, TRUE, 0);

                break;
	case GP_WIDGET_TEXT:
                if ((result = gp_widget_value_get (widget, &value_string)) != GP_OK) 
			g_warning (_("Could not get value for widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));

		/* Create the widget */                
		entry = gtk_entry_new ();
		gtk_widget_show (entry);
                if (value_string) gtk_entry_set_text (GTK_ENTRY (entry), value_string);
                gtk_signal_connect (GTK_OBJECT (entry), "changed", GTK_SIGNAL_FUNC (on_entry_changed), data);
                gtk_box_pack_end (GTK_BOX (hbox), entry, TRUE, TRUE, 0);

		break;
	case GP_WIDGET_DATE:
                if ((result = gp_widget_value_get (widget, &value_int)) != GP_OK)
			g_warning (_("Could not get value for widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));

		/* Create the widget */
		date_edit = gnome_date_edit_new ((time_t) value_int, TRUE, TRUE);
		gtk_signal_connect (GTK_OBJECT (date_edit), "date_changed", GTK_SIGNAL_FUNC (on_date_changed), data);
		gtk_signal_connect (GTK_OBJECT (date_edit), "time_changed", GTK_SIGNAL_FUNC (on_date_changed), data);
		gtk_widget_show (date_edit);
		gtk_box_pack_end (GTK_BOX (hbox), date_edit, TRUE, TRUE, 0);

		break;
	default:
		g_assert_not_reached ();
	}

	/* Create the control and add it to the component */
	control = bonobo_control_new (hbox);
	tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (widget));
	bonobo_ui_component_object_set (component, tmp, BONOBO_OBJREF (control), NULL);
	g_free (tmp);
}

static void
setup_commands (BonoboUIComponent* component, gchar* path, CameraWidget* widget, Camera* camera, gchar* folder, gchar* file)
{
	gchar*		tmp;
	gchar*		value_string = NULL;
	gint		j, result, value_int;
	WidgetData*	data;

	switch (gp_widget_type (widget)) {
	case GP_WIDGET_RADIO:
	case GP_WIDGET_MENU:
                if ((result = gp_widget_value_get (widget, &value_string)) != GP_OK) 
			g_warning (_("Could not get value for widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));

                for (j = 0; j < gp_widget_choice_count (widget); j++) {

                        /* Command */
                        tmp = g_strdup_printf (CMD_RADIO, gp_widget_id (widget), j, gp_widget_choice (widget, j),
                                gp_widget_choice (widget, j), gp_widget_label (widget));
                        bonobo_ui_component_set_translate (component, "/commands", tmp, NULL);
                        g_free (tmp);

                        /* Current state? */
			if (value_string) {
	                        tmp = g_strdup_printf ("/commands/%i_%i", gp_widget_id (widget), j);
        	                if (!strcmp (value_string, gp_widget_choice (widget, j))) {
                	                bonobo_ui_component_set_prop (component, tmp, "state", "1", NULL);
	                        } else {
        	                        bonobo_ui_component_set_prop (component, tmp, "state", "0", NULL);
                	        }
				g_free (tmp);
			}
                }

		/* Listener */
		for (j = 0; j < gp_widget_choice_count (widget); j++) {
			data = g_new0 (WidgetData, 1);
			data->camera = camera;
			data->widget = widget;
			data->folder = g_strdup (folder);
			data->file = g_strdup (file);
			tmp = g_strdup_printf ("%i_%i", gp_widget_id (widget), j);
			bonobo_ui_component_add_listener (component, tmp, on_togglebutton_clicked, data);
			g_free (tmp);
		}

                break;
        case GP_WIDGET_BUTTON:
                tmp = g_strdup_printf (CMD_BUTTON, gp_widget_id (widget), gp_widget_label (widget), gp_widget_label (widget));
                bonobo_ui_component_set_translate (component, "/commands", tmp, NULL);
                g_free (tmp);

                /* Verb */
                data = g_new0 (WidgetData, 1);
                data->camera = camera;
                data->widget = widget;
                data->folder = g_strdup (folder);
                data->file = g_strdup (file);
                tmp = g_strdup_printf ("%i", gp_widget_id (widget));
                bonobo_ui_component_add_verb (component, tmp, on_button_clicked, data);
                g_free (tmp);

                break;
        case GP_WIDGET_TOGGLE:
                value_int = 0;
                if ((result = gp_widget_value_get (widget, &value_int)) != GP_OK)
                        g_warning (_("Could not get value for widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
                tmp = g_strdup_printf (CMD_TOGGLE, gp_widget_id (widget), gp_widget_label (widget), gp_widget_label (widget), value_int);
                bonobo_ui_component_set_translate (component, "/commands", tmp, NULL);

                /* Listener */
                data = g_new0 (WidgetData, 1);
                data->camera = camera;
                data->folder = g_strdup (folder);
                data->file = g_strdup (file);
                data->widget = widget;
                tmp = g_strdup_printf ("%i", gp_widget_id (widget));
                bonobo_ui_component_add_listener (component, tmp, on_togglebutton_clicked, data);
                g_free (tmp);

                break;
	default:
		break;
	}
}

static void 
menu_prepare (Camera* camera, BonoboUIComponent* component, gchar* path, CameraWidget* widget, gchar* folder, gchar* file)
{
	CameraWidget*		child;
	gint 			i;
	gchar*			tmp;
	CameraWidgetType	type;

	for (i = 0; i < gp_widget_child_count (widget); i++) {
		child = gp_widget_child (widget, i);
		type = gp_widget_type (child);

		setup_menuitems (component, path, child);

	        if ((type == GP_WIDGET_RANGE) || (type == GP_WIDGET_DATE) || (type == GP_WIDGET_TEXT))
			setup_control (component, path, child, camera, folder, file);

		setup_commands (component, path, child, camera, folder, file);

		if (gp_widget_type (child) == GP_WIDGET_SECTION) {
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			menu_prepare (camera, component, tmp, child, folder, file);
			g_free (tmp);
		}
	}
}

void
menu_setup (BonoboUIComponent* component, Camera* camera, CameraWidget* widget, const gchar* path, gchar* folder, gchar* file)
{
        gchar*          tmp = NULL;

        bonobo_ui_component_set_translate (component, path, PLACEHOLDER, NULL);

        tmp = g_strconcat (path, "/Configuration/Configuration", NULL);
        menu_prepare (camera, component, tmp, widget, folder, file);
        g_free (tmp);
}

static GdkPixbuf*
scale_pixbuf (GdkPixbuf* pixbuf)
{
        GdkPixbuf*      pixbuf_scaled;

        pixbuf_scaled = gdk_pixbuf_new (
                gdk_pixbuf_get_colorspace (pixbuf), gdk_pixbuf_get_has_alpha (pixbuf), gdk_pixbuf_get_bits_per_sample (pixbuf),
                E_SHELL_MINI_ICON_SIZE, E_SHELL_MINI_ICON_SIZE);
        gdk_pixbuf_scale (pixbuf, pixbuf_scaled, 0, 0, E_SHELL_MINI_ICON_SIZE, E_SHELL_MINI_ICON_SIZE, 0.0, 0.0,
                (double) E_SHELL_MINI_ICON_SIZE / gdk_pixbuf_get_width (pixbuf),
                (double) E_SHELL_MINI_ICON_SIZE / gdk_pixbuf_get_height (pixbuf), GDK_INTERP_HYPER);
        return (pixbuf_scaled);
}

GdkPixbuf*
util_pixbuf_folder (void)
{
	if (!g_file_exists ("/usr/share/pixmaps/gnome-folder.png")) return (NULL);
	return (scale_pixbuf (gdk_pixbuf_new_from_file ("/usr/share/pixmaps/gnome-folder.png")));
}

GdkPixbuf*
util_pixbuf_file (void)
{
	if (!g_file_exists ("/usr/share/pixmaps/gnome-file-h.png")) return (NULL);
	return (scale_pixbuf (gdk_pixbuf_new_from_file ("/usr/share/pixmaps/gnome-file-h.png")));
}

