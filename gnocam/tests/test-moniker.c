#include <liboaf/liboaf.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-object.h>
#include <gtk/gtkmain.h>

int
main (int argc, char *argv [])
{
	CORBA_ORB orb;
	gchar *query;
	CORBA_Environment ev;
	Bonobo_Unknown object;

	gtk_type_init ();

	orb = oaf_init (argc, argv);

	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error ("Can not init bonobo!");

	CORBA_exception_init (&ev);

	g_message ("Activating from id...");
	object = oaf_activate_from_id ("OAFIID:Bonobo_Moniker_camera",
				       0, NULL, &ev);
	if (BONOBO_EX (&ev))
		g_error (bonobo_exception_get_text (&ev));
	if (object)
		bonobo_object_release_unref (object, NULL);
	else
		g_message ("... failed.");

	g_message ("Querying...");
	query = g_strdup_printf ("repo_ids.has ('IDL:Bonobo/Moniker:1.0') AND "
			         "bonobo:moniker.has ('camera:')");
	object = oaf_activate (query, NULL, 0, NULL, &ev);
	g_free (query);
	if (BONOBO_EX (&ev))
		g_error (bonobo_exception_get_text (&ev));
	if (object)
		bonobo_object_release_unref (object, NULL);
	else
		g_message ("... failed.");

	CORBA_exception_free (&ev);

	return (0);
}
