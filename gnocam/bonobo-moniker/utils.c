#include <config.h>
#include <gphoto2.h>
#include <parser.h>
#include <bonobo.h>

#include "gnocam-control.h"
#include "utils.h"

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

void menu_prepare       (CameraWidget* widget, xmlNodePtr popup, xmlNsPtr ns);
void menu_fill          (BonoboUIComponent* component, Camera* camera, const gchar* path, CameraWidget* window, CameraWidget* widget, gchar* folder, gchar* file);

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

	g_warning ("on_button_clicked");
	callback = gp_widget_callback (data->widget);
	if ((result = callback (data->camera, data->widget)) != GP_OK) 
		g_warning (_("Could not execute command '%s'! (%s)"), gp_widget_label (data->widget), gp_camera_result_as_string (data->camera, result));
}

static void
on_entry_changed (GtkObject* object, gpointer user_data)
{
	WidgetData*	data = (WidgetData*) gtk_object_get_data (object, "data");
	gchar*		value_string;
	gchar*		value_string_new;

	g_return_if_fail (data);

	g_return_if_fail (gp_widget_value_get (data->widget, &value_string) == GP_OK);
	value_string_new = gtk_entry_get_text (GTK_ENTRY (object));
	if (!value_string || (value_string && (strcmp (value_string, value_string_new) != 0))) {
		g_return_if_fail (gp_widget_value_set (data->widget, value_string_new) == GP_OK);
		set_config (data);
	}
}

static void
on_radio_button_activate (GtkObject* object, gpointer user_data)
{
	WidgetData*	data = (WidgetData*) gtk_object_get_data (object, "data");
	gchar*		value_string;
	gchar*		value_string_new;

	g_return_if_fail (data);

	g_return_if_fail (gp_widget_value_get (data->widget, &value_string) == GP_OK);
	value_string_new = gp_widget_choice (data->widget, GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (object), "choice")));
	if (!value_string || (value_string && (strcmp (value_string_new, value_string) != 0))) {
		g_return_if_fail (gp_widget_value_set (data->widget, value_string_new) == GP_OK);
		set_config (data);
	}
}

static void
on_adjustment_value_changed (GtkObject* object, gpointer user_data)
{
	WidgetData*	data = (WidgetData*) gtk_object_get_data (object, "data");
	gfloat		f, f_new;

	g_return_if_fail (data);

	g_return_if_fail (gp_widget_value_get (data->widget, &f) == GP_OK);
	f_new = GTK_ADJUSTMENT (object)->value;
	if (f != f_new) {
		g_return_if_fail (gp_widget_value_set (data->widget, &f_new) == GP_OK);
		set_config (data);
	}
}

static void
on_togglebutton_clicked (BonoboUIComponent* component, const char* path, Bonobo_UIComponent_EventType type, const char* state, gpointer user_data)
{
	WidgetData*     data = (WidgetData*) user_data;
	gint		i = atoi (state);
	
	g_return_if_fail (data);

	g_return_if_fail (gp_widget_value_set (data->widget, &i) == GP_OK);
	set_config (data);
}

static void
on_date_changed (GtkObject* object, gpointer user_data)
{
	WidgetData*	data = (WidgetData*) gtk_object_get_data (GTK_OBJECT (object), "data");
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

void
menu_setup (BonoboUIComponent* component, Camera* camera, CameraWidget* widget, const gchar* path, gchar* folder, gchar* file)
{
        gchar*          tmp = NULL;
        xmlDocPtr       doc = xmlNewDoc ("1.0");
        xmlNsPtr        ns = xmlNewGlobalNs (doc, "xxx", "xxx"); 
        xmlNodePtr      node, child;
        gint            i;

	node = xmlNewNode (ns, "placeholder");
	xmlSetProp (node, "name", "Configuration");
        xmlDocSetRootElement (doc, node);

	child = xmlNewNode (ns, "submenu");
	xmlSetProp (child, "name", "Configuration");
	xmlSetProp (child, "_label", "Configuration");
	xmlAddChild (node, child);
	
        menu_prepare (widget, child, ns);

        /* Send it to the component. */
        xmlDocDumpMemory (doc, (xmlChar**) &tmp, &i);
	printf ("DUMP:\n\n%s\n\n\n", tmp);
        xmlFreeDoc (doc);
        bonobo_ui_component_set_translate (component, path, tmp, NULL);
        g_free (tmp);

	/* Finish. */
	tmp = g_strconcat (path, "/Configuration/Configuration", NULL);
	menu_fill (component, camera, tmp, widget, widget, folder, file);
	g_free (tmp);
}

void 
menu_prepare (CameraWidget* widget, xmlNodePtr menu, xmlNsPtr ns)
{
	CameraWidget*		child;
	gint 			i, value_int;
	xmlNodePtr		node;
	gchar*			id;
	gchar*			tmp;

	for (i = 0; i < gp_widget_child_count (widget); i++) {
		child = gp_widget_child (widget, i);
		id = g_strdup_printf ("%i", gp_widget_id (child));
		switch (gp_widget_type (child)) {
		case GP_WIDGET_WINDOW:
			break;
		case GP_WIDGET_SECTION:
			xmlAddChild (menu, node = xmlNewNode (ns, "submenu"));
			xmlSetProp (node, "name", id);
			xmlSetProp (node, "_label", gp_widget_label (child));
			xmlSetProp (node, "_tip", gp_widget_label (child));
			menu_prepare (child, node, ns);
			break;
		case GP_WIDGET_TEXT:
		case GP_WIDGET_MENU:
		case GP_WIDGET_DATE:
		case GP_WIDGET_RADIO:
		case GP_WIDGET_RANGE:
			xmlAddChild (menu, node = xmlNewNode (ns, "control"));
			xmlSetProp (node, "name", id);
			xmlSetProp (node, "_tip", gp_widget_label (child));
			break;
		case GP_WIDGET_BUTTON:
			xmlAddChild (menu, node = xmlNewNode (ns, "menuitem"));
			xmlSetProp (node, "name", id);
			xmlSetProp (node, "_tip", gp_widget_label (child));
			xmlSetProp (node, "_label", gp_widget_label (child));
			xmlSetProp (node, "verb", id);
			break;
		case GP_WIDGET_TOGGLE:
			xmlAddChild (menu, node = xmlNewNode (ns, "menuitem"));
			xmlSetProp (node, "name", id);
			xmlSetProp (node, "_tip", gp_widget_label (child));
			xmlSetProp (node, "_label", gp_widget_label (child));
			xmlSetProp (node, "type", "toggle");
			xmlSetProp (node, "verb", id);
			if (gp_widget_value_get (child, &value_int) == GP_OK) {
				tmp = g_strdup_printf ("%i", value_int);
				xmlSetProp (node, "state", tmp);
				g_free (tmp);
			}
			break;
		default:
			g_warning ("Encountered unsupported widget!");
			break;
		}
		g_free (id);
	}
}

void 
menu_fill (BonoboUIComponent* component, Camera* camera, const gchar* path, CameraWidget* window, CameraWidget* widget, gchar* folder, gchar* file)
{
	GtkWidget*		hbox;
	GtkWidget*		gtkwidget;
	GtkWidget*		menu;
	GtkWidget*		menu_item;
	GtkObject*		adjustment;
	CameraWidget*		child;
	gint			i, j;
	gchar*			tmp;
	gchar*			value_string;
	gfloat			max, min, increment, value_float;
	gint			value_int;
	WidgetData*		data;

	for (i = 0; i < gp_widget_child_count (widget); i++) {
		child = gp_widget_child (widget, i);
		switch (gp_widget_type (child)) {
		case GP_WIDGET_WINDOW:
			break;
		case GP_WIDGET_SECTION:
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			menu_fill (component, camera, tmp, window, child, folder, file);
			g_free (tmp);
			break;
		case GP_WIDGET_MENU:
		case GP_WIDGET_RADIO:
			gp_widget_value_get (child, &value_string);
			gtk_widget_show (hbox = gtk_hbox_new (FALSE, 5));
			gtk_widget_show (gtkwidget = gtk_label_new (gp_widget_label (child)));
			gtk_box_pack_start (GTK_BOX (hbox), gtkwidget, FALSE, FALSE, 0);
			gtk_widget_show (gtkwidget = gtk_option_menu_new ());
			gtk_widget_show (menu = gtk_menu_new ());
			gtk_option_menu_set_menu (GTK_OPTION_MENU (gtkwidget), menu);
			for (j = 0; j < gp_widget_choice_count (child); j++) {
				gtk_widget_show (menu_item = gtk_menu_item_new_with_label (gp_widget_choice (child, j)));
				gtk_menu_append (GTK_MENU (menu), menu_item);
				data = g_new0 (WidgetData, 1);
	                        data->camera = camera;
	                        data->widget = child;
	                        data->folder = g_strdup (folder);
	                        data->file = g_strdup (file);
	                        gtk_object_set_data (GTK_OBJECT (menu_item), "data", data);
				gtk_object_set_data (GTK_OBJECT (menu_item), "choice", GINT_TO_POINTER (j));
				if (value_string && (strcmp (value_string, gp_widget_choice (child, j)) == 0)) 
					gtk_option_menu_set_history (GTK_OPTION_MENU (gtkwidget), j);
				gtk_signal_connect (GTK_OBJECT (menu_item), "activate", GTK_SIGNAL_FUNC (on_radio_button_activate), NULL);
			}
			gtk_box_pack_end (GTK_BOX (hbox), gtkwidget, TRUE, TRUE, 0);
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			bonobo_ui_component_object_set (component, tmp, bonobo_object_corba_objref (BONOBO_OBJECT (bonobo_control_new (hbox))), NULL);
			g_free (tmp);
			break;
		case GP_WIDGET_TOGGLE:
			data = g_new0 (WidgetData, 1);
			data->camera = camera;
			data->folder = g_strdup (folder);
			data->file = g_strdup (file);
			data->widget = child;
			tmp = g_strdup_printf ("%i", gp_widget_id (child));
			bonobo_ui_component_add_listener (component, tmp, on_togglebutton_clicked, data);
                        g_free (tmp);
			break;
		case GP_WIDGET_RANGE:
			gp_widget_value_get (child, &value_float);
			gp_widget_range_get (child, &min, &max, &increment);
			gtk_widget_show (hbox = gtk_hbox_new (FALSE, 5));
			gtk_widget_show (gtkwidget = gtk_label_new (gp_widget_label (child)));
			gtk_box_pack_start (GTK_BOX (hbox), gtkwidget, FALSE, FALSE, 0);
			adjustment = gtk_adjustment_new (value_float, min, max, increment, 0, 0);
			data = g_new0 (WidgetData, 1);
                        data->camera = camera;
                        data->widget = child;
                        data->folder = g_strdup (folder);
                        data->file = g_strdup (file);
                        gtk_object_set_data (GTK_OBJECT (adjustment), "data", data);
			gtk_signal_connect (adjustment, "value_changed", GTK_SIGNAL_FUNC (on_adjustment_value_changed), NULL);
			gtk_widget_show (gtkwidget = gtk_hscale_new (GTK_ADJUSTMENT (adjustment)));
			gtk_range_set_update_policy (GTK_RANGE (gtkwidget), GTK_UPDATE_DISCONTINUOUS);
			gtk_box_pack_end (GTK_BOX (hbox), gtkwidget, TRUE, TRUE, 0);
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			bonobo_ui_component_object_set (component, tmp, bonobo_object_corba_objref (BONOBO_OBJECT (bonobo_control_new (hbox))), NULL);
			g_free (tmp);
			break;
		case GP_WIDGET_DATE:
			gp_widget_value_get (child, &value_int);
			gtk_widget_show (hbox = gtk_hbox_new (FALSE, 5));
			gtk_widget_show (gtkwidget = gtk_label_new (gp_widget_label (child)));
			gtk_box_pack_start (GTK_BOX (hbox), gtkwidget, FALSE, FALSE, 0);
			gtk_widget_show (gtkwidget = gnome_date_edit_new ((time_t) value_int, TRUE, TRUE));
			data = g_new0 (WidgetData, 1);
                        data->camera = camera;
                        data->widget = child;
                        data->folder = g_strdup (folder);
                        data->file = g_strdup (file);
                        gtk_object_set_data (GTK_OBJECT (gtkwidget), "data", data);
			gtk_signal_connect (GTK_OBJECT (gtkwidget), "date_changed", GTK_SIGNAL_FUNC (on_date_changed), NULL);
			gtk_signal_connect (GTK_OBJECT (gtkwidget), "time_changed", GTK_SIGNAL_FUNC (on_date_changed), NULL);
			gtk_box_pack_end (GTK_BOX (hbox), gtkwidget, TRUE, TRUE, 0);
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			bonobo_ui_component_object_set (component, tmp, bonobo_object_corba_objref (BONOBO_OBJECT (bonobo_control_new (hbox))), NULL);
			g_free (tmp);
			break;
		case GP_WIDGET_TEXT:
			gp_widget_value_get (child, &value_string);
			gtk_widget_show (hbox = gtk_hbox_new (FALSE, 5));
			gtk_widget_show (gtkwidget = gtk_label_new (gp_widget_label (child)));
			gtk_box_pack_start (GTK_BOX (hbox), gtkwidget, FALSE, FALSE, 0);
			gtk_widget_show (gtkwidget = gtk_entry_new ());
			if (value_string) gtk_entry_set_text (GTK_ENTRY (gtkwidget), value_string);
			data = g_new0 (WidgetData, 1);
			data->camera = camera;
			data->widget = child;
			data->folder = g_strdup (folder);
			data->file = g_strdup (file);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "data", data);
			gtk_signal_connect (GTK_OBJECT (gtkwidget), "changed", GTK_SIGNAL_FUNC (on_entry_changed), NULL);
			gtk_box_pack_end (GTK_BOX (hbox), gtkwidget, TRUE, TRUE, 0);
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			bonobo_ui_component_object_set (component, tmp, bonobo_object_corba_objref (BONOBO_OBJECT (bonobo_control_new (hbox))), NULL);
			g_free (tmp);
			break;
		case GP_WIDGET_BUTTON:
			data = g_new0 (WidgetData, 1);
			data->camera = camera;
			data->widget = child;
			data->folder = g_strdup (folder);
			data->file = g_strdup (file);
			tmp = g_strdup_printf ("%i", gp_widget_id (child));
			bonobo_ui_component_add_verb (component, tmp, on_button_clicked, data);
			g_free (tmp);
			break;
		default:
			g_warning ("Encountered unsupported widget!");
			break;
		}
	}
}



