#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <gnome.h>
#include <gphoto2/gphoto2.h>
#include "preferences.h"
#include "gphoto-extensions.h"

/**
 * preferences_read:
 * @capplet: Pointer to the capplet.
 *
 * Reads the current settings and populates the capplet.
 **/
void preferences_read (GtkWidget *capplet)
{
	GtkCList *clist;
	gchar *prefix;
	gint i = 0;
	gboolean def;
	Camera *camera;
	gchar *text[4];

        clist = GTK_CLIST (gtk_object_get_data (GTK_OBJECT (capplet), "clist"));

	prefix = g_strdup_printf ("/gphoto/Camera %i/", i);
	gnome_config_push_prefix (prefix);

	/* Check if we've got entries. */
	text[0] = gnome_config_get_string_with_default ("name", &def);
	if (!def) {

		/* We've got at least one entry. */
		while (!def) {

			/* Read this entry. */
			text[1] = gnome_config_get_string ("model"); 
			text[2] = gnome_config_get_string ("port");
			text[3] = gnome_config_get_string ("speed");
			if ((camera = gp_camera_new_by_description (text[1], text[2], text[3])) == NULL) {
				gnome_error_dialog (_("The configuration file is corrupt!"));
			} else {

				/* Add camera to clist. */
				gtk_clist_append (clist, text);
				gtk_clist_set_row_data (clist, clist->rows - 1, camera);
			}

			/* Go to next entry. */
			gnome_config_pop_prefix ();
			g_free (prefix);
			prefix = g_strdup_printf ("/gphoto/Camera %i/", ++i);
			gnome_config_push_prefix (prefix);
			text[0] = gnome_config_get_string_with_default ("name", &def); 
		}
	}

	/* Clean up. */
	g_free (prefix);
	gnome_config_pop_prefix ();
}

/**
 * preferences_write:
 * @capplet: Pointer to the capplet.
 *
 * Reads the entries in the capplet and writes them to file.
 **/
void preferences_write (GtkWidget *capplet)
{
	GtkCList *clist;
	gint i;
	gchar *prefix;
        gchar *name, *model, *speed, *port;
	gboolean def = FALSE;

	clist = GTK_CLIST (gtk_object_get_data (GTK_OBJECT (capplet), "clist"));

	for (i = 0; i < clist->rows; i++) {

		/* Get the entries of the clist. */
                gtk_clist_get_text (clist, i, 0, &name);
                gtk_clist_get_text (clist, i, 1, &model);
                gtk_clist_get_text (clist, i, 2, &port);
                gtk_clist_get_text (clist, i, 3, &speed);
	
		/* Write those entries to the configuration file. */
                prefix = g_strdup_printf ("/gphoto/Camera %i/", i);
                gnome_config_push_prefix (prefix);
                gnome_config_set_string ("name", name);
                gnome_config_set_string ("model", model);
                gnome_config_set_string ("port", port);
                gnome_config_set_string ("speed", speed);
		gnome_config_pop_prefix ();
		g_free (prefix);
	}
	
	/* Check if we have to clean up. */
        prefix = g_strdup_printf ("/gphoto/Camera %i/", i++);
        gnome_config_push_prefix (prefix);
        gnome_config_get_string_with_default ("model", &def);
        if (!def) {

                /* We have to clean up. */
                while (!def) {
                        gnome_config_clean_section (prefix);
                        g_free (prefix);
                        gnome_config_pop_prefix ();
                        prefix = g_strdup_printf ("/gphoto/Camera %i/", i++);
                        gnome_config_push_prefix (prefix);
                        gnome_config_get_string_with_default ("model", &def);
                }
        }
        g_free (prefix);
        gnome_config_pop_prefix ();
        gnome_config_sync ();

}
