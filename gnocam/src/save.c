#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include "save.h"
#include "gnocam.h"

/**
 * save:
 * @xml: A reference to the xml of the main window.
 * file: Save selected files (TRUE) or previews (FALSE)?
 * temp: Save temporarily (TRUE) or in prefix?
 *
 * Saves the selected files/previews.
 **/
void 
save (GladeXML *xml, gboolean file, gboolean temp)
{
        GList *selection;
        Camera *camera;
        CameraFile *camera_file = NULL;
        GtkCList *clist;
        GnomeApp *app;
        gchar *path, *file_name, *full_file_name;
        guint i, row;
        gint return_status = GP_OK;

        g_assert (xml != NULL);
        clist = GTK_CLIST (glade_xml_get_widget (xml, "clist_files"));
        g_assert (clist != NULL);
        app = GNOME_APP (glade_xml_get_widget (xml, "app"));
        g_assert (app != NULL);

        /* Check which files have been selected. */
        selection = g_list_first (clist->selection);
        if (selection != NULL) {
                camera_file = gp_file_new ();
                for (i = 0; i < g_list_length (selection); i++) {

                        /* Retrieve some data we need. */
                        row = GPOINTER_TO_INT (g_list_nth_data (selection, i));
                        camera = gtk_clist_get_row_data (clist, row);
                        g_assert (camera != NULL);                        gtk_clist_get_text (clist, row, 1, &path);
                        g_assert (path != NULL);
                        gtk_clist_get_text (clist, row, 2, &file_name);
                        g_assert (file_name != NULL);

                        /* File or preview? */
                        if (file) 
                                return_status = gp_camera_file_get (camera, camera_file, path, file_name);
                        else
                                return_status = gp_camera_file_get_preview (camera, camera_file, path, file_name);
                        if (return_status == GP_ERROR) gnome_app_error (app, _("Could not get file from camera!"));
			else {
				
				/* Let the back-end save the file... */
				if (temp) {
					full_file_name = g_strdup_printf ("/tmp/%s", file_name);
				} else { 
					//FIXME: Quick hack. Should be done better...
					full_file_name = g_strdup_printf ("%s%s", (gchar *) gtk_object_get_data (GTK_OBJECT (app), "prefix"), file_name);
				}
				gp_file_save (camera_file, full_file_name);
			}

			/* Clean up. */
			gp_interface_progress (camera, NULL, 0.0);
                }
                g_free (camera_file);
        } else {
                gnome_app_error (app, _("No files selected!"));
        }
}

