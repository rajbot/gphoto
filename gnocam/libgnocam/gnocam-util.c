#include <config.h>
#include "gnocam-util.h"

#include <string.h>

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-object.h>

#include <bonobo-activation/bonobo-activation-activate.h>

GNOME_C_Camera
gnocam_util_get_camera (const gchar *manuf, const gchar *model,
			const gchar *port, CORBA_Environment *ev)
{
	Bonobo_ServerInfoList *l;
        GNOME_C_Mngr m;
        GNOME_C_Mngr_ManufacturerList *ml;
        GNOME_C_Mngr_ModelList model_l;
        GNOME_C_Mngr_PortList pl;
        guint i, j, k, n;
	GNOME_C_Camera c;

	if (!manuf || !model || !port) {
		CORBA_exception_set (ev, CORBA_SYSTEM_EXCEPTION,
				     ex_CORBA_BAD_PARAM, NULL);
		return CORBA_OBJECT_NIL;
	}

        l = bonobo_activation_query ("repo_ids.has('IDL:GNOME/C/Mngr')", NULL,
                                     ev);
        if (BONOBO_EX (ev)) return CORBA_OBJECT_NIL;
        for (i = 0; i < l->_length; i++) {
            m = bonobo_get_object (l->_buffer[i].iid, "IDL:GNOME/C/Mngr", ev);
            if (BONOBO_EX (ev)) {
		CORBA_exception_set (ev, CORBA_NO_EXCEPTION, NULL, NULL);
		continue;
	    }
            ml = GNOME_C_Mngr__get_devices (m, ev);
            if (BONOBO_EX (ev)) {
                    bonobo_object_release_unref (m, NULL);
		    CORBA_exception_set (ev, CORBA_NO_EXCEPTION, NULL, NULL);
                    continue;
            }
            for (j = 0; j < ml->_length; j++) {
                if (strcmp (manuf, ml->_buffer[j].manufacturer))
                    continue;
                model_l = ml->_buffer[j].models;
                for (k = 0; k < model_l._length; k++) {
                    if (strcmp (model, model_l._buffer[k].model))
                        continue;
                    pl = model_l._buffer[k].ports;
                    for (n = 0; n < pl._length; n++)
                        if (!strcmp (port, pl._buffer[n])) {
                            c = GNOME_C_Mngr_connect (m, manuf, model,
					    	      port, ev);
                            bonobo_object_release_unref (m, NULL);
                            CORBA_free (ml);
                            CORBA_free (l);
			    if (BONOBO_EX (ev)) {
				    /* bonobo_object_release_unref (c, NULL); */
				    return CORBA_OBJECT_NIL;
			    }
                            return c;
                        }
                }
            }
            bonobo_object_release_unref (m, NULL);
            CORBA_free (ml);
        }
        CORBA_free (l);

	CORBA_exception_set (ev, CORBA_SYSTEM_EXCEPTION,
			     ex_CORBA_UNKNOWN, NULL);
	return CORBA_OBJECT_NIL;
}
