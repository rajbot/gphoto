#include "config.h"

#include <gphoto2.h>
#include <liboaf/liboaf.h>
#include <bonobo/bonobo-main.h>

#include <gconf/gconf.h>
#include <capplet-widget.h>

#include "gnocam-capplet-content.h"

/***************/
/* Log handler */
/***************/

static void
log_handler (const gchar *log_domain, GLogLevelFlags log_level,
	     const gchar *message, gpointer user_data)
{
        GtkWindow *window;
        
        window = GTK_WINDOW (user_data);

        if (log_level == G_LOG_LEVEL_CRITICAL)
		gnome_error_dialog_parented (message, window);
        else if (log_level == G_LOG_LEVEL_WARNING)
		gnome_warning_dialog_parented (message, window);
        else if ((log_level == G_LOG_LEVEL_INFO) ||
		 (log_level == G_LOG_LEVEL_MESSAGE))
		gnome_ok_dialog_parented (message, window);
}

/*************/
/* Callbacks */
/*************/

static void
on_try (CappletWidget* widget, gpointer user_data)
{
	GnoCamCappletContent*   content;
	
	content = GNOCAM_CAPPLET_CONTENT (user_data);
	
	gnocam_capplet_content_try (content);
}

static void
on_revert (CappletWidget* widget, gpointer user_data)
{
	GnoCamCappletContent*   content;
	
	content = GNOCAM_CAPPLET_CONTENT (user_data);
	
	gnocam_capplet_content_revert (content);
}

static void
on_ok (CappletWidget* widget, gpointer user_data)
{
	GnoCamCappletContent*	content;
	
	content = GNOCAM_CAPPLET_CONTENT (user_data);

	gnocam_capplet_content_ok (content);
}

static void
on_cancel (CappletWidget* widget, gpointer user_data)
{
	GnoCamCappletContent*   content;

	content = GNOCAM_CAPPLET_CONTENT (user_data);
	
	gnocam_capplet_content_cancel (content);
}

/*************/
/* Functions */
/*************/

int main (int argc, char** argv)
{
	GError *error;
	GtkWidget *content;
	GtkWidget *capplet;
	CORBA_ORB orb;
	
	/* Internationalization */
	bindtextdomain (PACKAGE, GNOME_LOCALEDIR);
	textdomain (PACKAGE);

	gtk_type_init ();

	orb = oaf_init (argc, argv);
	if (!bonobo_init (orb, NULL, NULL))
		g_error ("Can not initialize bonobo!");

	if (gnome_capplet_init (PACKAGE, VERSION, argc, argv, NULL, 0, NULL) < 0) return (1);

	if (!gconf_init(argc, argv, &error)) {
		g_assert(error);
		g_warning("GConf init failed: %s!", error->message);
		return (1);
	}

	/* Create the capplet */
	capplet = capplet_widget_new ();
	gtk_widget_show (capplet);

	/* Redirect messages */
	g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING |
			   G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_MESSAGE |
			   G_LOG_LEVEL_INFO, log_handler, capplet);

	/* Create the capplet's contents */
	content = gnocam_capplet_content_new (CAPPLET_WIDGET (capplet));
	gtk_widget_show (content);
	gtk_container_add (GTK_CONTAINER (capplet), content);

	/* Connect the signals */
	gtk_signal_connect (GTK_OBJECT (capplet), "try",
			    GTK_SIGNAL_FUNC (on_try), content); 
	gtk_signal_connect (GTK_OBJECT (capplet), "revert",
			    GTK_SIGNAL_FUNC (on_revert), content); 
	gtk_signal_connect (GTK_OBJECT (capplet), "ok",
			    GTK_SIGNAL_FUNC (on_ok), content); 
	gtk_signal_connect (GTK_OBJECT (capplet), "cancel",
			    GTK_SIGNAL_FUNC (on_cancel), content);
	
	capplet_gtk_main ();

	return (0);
}

