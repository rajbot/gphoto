#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#include <bonobo.h>
#include "gnocam.h"
#include "cameras.h"
#include "notification.h"

/**********************/
/* External Variables */
/**********************/

extern GtkWindow*		main_window;
extern GtkTree*			main_tree;
extern GladeXML*		xml_main;
extern BonoboObjectClient*	viewer_client;
extern BonoboUIContainer*	container;

/*************/
/* Functions */
/*************/

void
on_camera_setup_changed (GConfClient* client, guint notify_id, GConfEntry* entry, gpointer user_data)
{
	main_tree_update (entry->value);
}

void
on_viewer_changed (GConfClient* client, guint notify_id, GConfEntry* entry, gpointer user_data)
{
	GtkWidget*	viewer;
	GtkPaned*	paned;
	
	/* If there is an old viewer, destroy it. */
	g_return_if_fail (paned = GTK_PANED (glade_xml_get_widget (xml_main, "main_hpaned")));
	if (paned->child2) gtk_container_remove (GTK_CONTAINER (paned), paned->child2);

	/* Create the new viewer. */
	if (entry->value) {
		if ((viewer = bonobo_widget_new_control ((gchar*) gconf_value_get_string (entry->value), bonobo_object_corba_objref (BONOBO_OBJECT (container))))) {
			gtk_widget_show (viewer);
			gtk_paned_pack2 (GTK_PANED (glade_xml_get_widget (xml_main, "main_hpaned")), viewer, TRUE, TRUE);
			viewer_client = bonobo_widget_get_server (BONOBO_WIDGET (viewer));
		} else gnome_error_dialog_parented (_("Could not start the image viewer!"), main_window);
	}
}


