#include <config.h>

#include <bonobo-activation/bonobo-activation-activate.h>

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-main.h>

#include "GnoCam.h"

#define CHECK(ev) {if (BONOBO_EX (ev)) g_error (bonobo_exception_get_text (ev));}

int
main (int argc, char *argv[])
{
	CORBA_Object gnocam;
	CORBA_Environment ev;
	GNOME_GnoCam_Camera camera;
	GNOME_GnoCam_Factory_ModelList *model_list;
	int i;

	bonobo_init (&argc, argv);

	CORBA_exception_init (&ev);

	g_message ("Getting GnoCam...");
	gnocam = bonobo_activation_activate_from_id (
			"OAFIID:GNOME_GnoCam", 0, NULL, &ev);
	CHECK (&ev);

	g_message ("Getting list of models...");
	model_list = GNOME_GnoCam_Factory_getModelList (gnocam, &ev);
	CHECK (&ev);

	g_message ("... done. Got the following model(s):");
	for (i = 0; i < model_list->_length; i++)
		g_message ("  %2i: '%s'", i, model_list->_buffer[i]);
	CORBA_free (model_list);

	g_message ("Getting camera 'Directory Browse'...");
	camera = GNOME_GnoCam_Factory_getCameraByModel (gnocam,
					"Directory Browse", &ev);
	bonobo_object_release_unref (gnocam, NULL);
	CHECK (&ev);

	bonobo_object_release_unref (camera, NULL);

	CORBA_exception_free (&ev);

	return (0);
}
