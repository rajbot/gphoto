#include <config.h>
#include "knc-c-mngr.h"
#include "knc-c-camera.h"

#include <gphoto2-port-info-list.h>

#include <libknc/knc-utils.h>

#include <bonobo/bonobo-exception.h>

struct _KncCMngrPriv {

	GPPortInfoList *il;
};

static GObjectClass *parent_class;

static CORBA_string
impl_get_port_name (PortableServer_Servant servant, GNOME_C_ID n,
		    CORBA_Environment *ev)
{
	KncCMngr *m = KNC_C_MNGR (bonobo_object (servant));
	GPPortInfo info;
	unsigned int i, c;

	for (i = c = 0; (i < gp_port_info_list_count (m->priv->il)) &&
			(c <= n); i++) {
		gp_port_info_list_get_info (m->priv->il, i, &info);
		if (info.type == GP_PORT_SERIAL) c++;
		if (n + 1 == c) return CORBA_string_dup (info.name);
	}
	return NULL;
}

static GNOME_C_IDList *
impl_get_devices (PortableServer_Servant servant, CORBA_Environment *ev)
{
	GNOME_C_IDList *l;
	unsigned int i;

	l = GNOME_C_IDList__alloc ();
	l->_length = knc_count_devices ();
	l->_buffer = CORBA_sequence_CORBA_unsigned_long_allocbuf (l->_length);
	for (i = 0; i < l->_length; i++) l->_buffer[i] = i;
	CORBA_sequence_set_release (l, CORBA_TRUE);

	return l;
}

static GNOME_C_Mngr_Device *
impl_get_device (PortableServer_Servant servant, GNOME_C_ID id,
		 CORBA_Environment *ev)
{
	KncCMngr *m = KNC_C_MNGR (bonobo_object (servant));
	GNOME_C_Mngr_Device *d;
	GPPortInfo info;
	unsigned int i, n;

	if (id >= knc_count_devices ()) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_GNOME_C_BadID, NULL);
		return NULL;
	}

	d = GNOME_C_Mngr_Device__alloc ();
	d->manufacturer = CORBA_string_dup (knc_get_device_manufacturer (id));
	d->model = CORBA_string_dup (knc_get_device_model (id));

	for (i = n = 0; i < gp_port_info_list_count (m->priv->il); i++) {
		gp_port_info_list_get_info (m->priv->il, i, &info);
		if (info.type == GP_PORT_SERIAL) n++;
	}
	d->ports._length = n;
	d->ports._buffer = CORBA_sequence_CORBA_unsigned_long_allocbuf (
							d->ports._length);
	for (i = 0; i < d->ports._length; i++) d->ports._buffer[i] = i;

	return d;
}

static GNOME_C_Camera
impl_connect_to_device_at_port (PortableServer_Servant servant,
		GNOME_C_ID d, GNOME_C_ID p, CORBA_Environment *ev)
{
	KncCMngr *m = KNC_C_MNGR (bonobo_object (servant));
	KncCCamera *c;
	GPPort *port = NULL;
	GPPortInfo info;
	unsigned int i, n;

	gp_port_new (&port);
	for (i = n = 0; (i < gp_port_info_list_count (m->priv->il)) &&
			(n <= p); i++) {
		gp_port_info_list_get_info (m->priv->il, i, &info);
		if (info.type == GP_PORT_SERIAL) n++;
		if (p + 1 == n) {
			gp_port_set_info (port, info);
			c = knc_c_camera_new (knc_get_device_manufacturer (d),
				knc_get_device_model (d), port, ev);
			if (BONOBO_EX (ev)) return CORBA_OBJECT_NIL;
			return CORBA_Object_duplicate (BONOBO_OBJREF (c), ev);
		}
	}
	gp_port_free (port);
	return NULL;
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
	epv->get_device = impl_get_device;
	epv->get_port_name = impl_get_port_name;
	epv->connect_to_device_at_port = impl_connect_to_device_at_port;

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
