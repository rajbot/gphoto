#include <config.h>
#include "knc-c-mngr.h"
#include "knc-c-camera.h"

#include <string.h>

#include <gphoto2-port-info-list.h>

#include <libknc/knc-utils.h>

#include <bonobo/bonobo-exception.h>

struct _KncCMngrPriv {

	GPPortInfoList *il;
};

static GObjectClass *parent_class;

static GNOME_C_Mngr_ManufacturerList *
impl_get_devices (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCMngr *m = KNC_C_MNGR (bonobo_object (servant));
	GNOME_C_Mngr_ManufacturerList *l;
	unsigned int i, j, k, n, o;
	GPPortInfo info;
	int c;

	l = GNOME_C_Mngr_ManufacturerList__alloc ();

	/* List all manufacturers */
	l->_buffer = GNOME_C_Mngr_ManufacturerList_allocbuf (
						knc_count_devices ());
	l->_length = 0;
	for (i = 0; i < knc_count_devices (); i++) {
	    for (j = 0; j < l->_length; j++)
		if (!strcmp (l->_buffer[j].manufacturer,
			     knc_get_device_manufacturer (i))) break;
	    if (j == l->_length) {
		l->_length++;
		l->_buffer[j].manufacturer = CORBA_string_dup (
				knc_get_device_manufacturer (i));
		l->_buffer[j].models._length = 0;
		l->_buffer[j].models._buffer =
			GNOME_C_Mngr_ModelList_allocbuf (knc_count_devices ());

		/* List all models */
		for (k = 0; k < knc_count_devices (); k++) {
		    if (strcmp (l->_buffer[j].manufacturer,
			        knc_get_device_manufacturer (k))) continue;
		    n = l->_buffer[j].models._length;
		    l->_buffer[j].models._length++;
		    l->_buffer[j].models._buffer[n].model =
			    CORBA_string_dup (knc_get_device_model (k));

		    /* List all serial ports */
		    c = gp_port_info_list_count (m->priv->il);
		    l->_buffer[j].models._buffer[n].ports._length = 0;
		    l->_buffer[j].models._buffer[n].ports._buffer =
			    CORBA_sequence_CORBA_string_allocbuf (MAX (c, 0));
		    for (o = 0; o < MAX (c, 0); o++) {
			gp_port_info_list_get_info (m->priv->il, o, &info);
			if (info.type == GP_PORT_SERIAL) {
			    l->_buffer[j].models._buffer[n].ports._buffer[
				l->_buffer[j].models._buffer[n].ports._length] =
					CORBA_string_dup (info.name);
			    l->_buffer[j].models._buffer[n].ports._length++;
			}
		    }
		}
	    }
	}
	CORBA_sequence_set_release (l, CORBA_TRUE);

	return l;
}

static GNOME_C_Camera
impl_connect (PortableServer_Servant servant, const CORBA_char *manufacturer,
	      const CORBA_char *model, const CORBA_char *port, 
	      CORBA_Environment *ev)
{
	KncCMngr *m = KNC_C_MNGR (bonobo_object (servant));
	KncCCamera *c;
	GPPort *p = NULL;
	GPPortInfo info;
	unsigned int i;

	gp_port_new (&p);
	for (i = 0; i < MAX (gp_port_info_list_count (m->priv->il), 0); i++) {
		gp_port_info_list_get_info (m->priv->il, i, &info);
		if (!strcmp (info.name, port)) {
			gp_port_set_info (p, info);
			c = knc_c_camera_new (manufacturer, model, p, ev);
			if (BONOBO_EX (ev)) return CORBA_OBJECT_NIL;
			return CORBA_Object_duplicate (BONOBO_OBJREF (c), ev);
		}
	}
	gp_port_free (p);
	
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_GNOME_C_Error, NULL);
	return CORBA_OBJECT_NIL;
}

static void
knc_c_mngr_finalize (GObject *o)
{
	KncCMngr *m = KNC_C_MNGR (o);

	gp_port_info_list_free (m->priv->il);
	g_free (m->priv);

	G_OBJECT_CLASS (parent_class)->finalize (o);
}

static void
knc_c_mngr_class_init (KncCMngrClass *klass)
{
	POA_GNOME_C_Mngr__epv *epv = &klass->epv;
	GObjectClass *g_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	epv->get_devices = impl_get_devices;
	epv->connect = impl_connect;

	g_class->finalize = knc_c_mngr_finalize;
}

static void
knc_c_mngr_init (KncCMngr *m)
{
	m->priv = g_new0 (KncCMngrPriv, 1);

	gp_port_info_list_new (&m->priv->il);
	gp_port_info_list_load (m->priv->il);
}

BONOBO_TYPE_FUNC_FULL (KncCMngr, GNOME_C_Mngr, BONOBO_TYPE_OBJECT, knc_c_mngr);
