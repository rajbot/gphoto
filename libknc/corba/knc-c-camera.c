#include <config.h>
#include "knc-c-camera.h"
#include "knc-c-dir.h"
#include "knc-c-bag.h"

#include <libknc/knc-cntrl.h>
#include <libknc/knc.h>

struct _KncCCameraPriv {
	KncCntrl *c;
	gchar *manufacturer, *model;
};

static GObjectClass *parent_class;

static void
knc_c_camera_finalize (GObject *o)
{
	KncCCamera *c = KNC_C_CAMERA (o);

	knc_cntrl_unref (c->priv->c);
	g_free (c->priv->manufacturer);
	g_free (c->priv->model);
	g_free (c->priv);

	G_OBJECT_CLASS (parent_class)->finalize (o);
}

static CORBA_string
impl_get_manufacturer (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCCamera *c = KNC_C_CAMERA (bonobo_object (servant));

	return CORBA_string_dup (c->priv->manufacturer);
}

static CORBA_string
impl_get_model (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCCamera *c = KNC_C_CAMERA (bonobo_object (servant));

	return CORBA_string_dup (c->priv->model);
}

static GNOME_C_Dir
impl_get_dir (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCCamera *c = KNC_C_CAMERA (bonobo_object (servant));
	KncCDir *d = knc_c_dir_new (c->priv->c);

	return CORBA_Object_duplicate (BONOBO_OBJREF (d), ev);
}

static GNOME_C_Bag
impl_get_bag (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCCamera *c = KNC_C_CAMERA (bonobo_object (servant));
	KncCBag *b = knc_c_bag_new (c->priv->c, KNC_C_BAG_TYPE_ROOT);

	return CORBA_Object_duplicate (BONOBO_OBJREF (b), ev);
}

static void
knc_c_camera_class_init (KncCCameraClass *klass)
{
	POA_GNOME_C_Camera__epv *epv = &klass->epv;
	GObjectClass *g_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	epv->_get_manufacturer = impl_get_manufacturer;
	epv->_get_model        = impl_get_model;
	epv->get_dir           = impl_get_dir;
	epv->get_bag           = impl_get_bag;

	g_class->finalize = knc_c_camera_finalize;
}

static KncCntrlRes
read_func (unsigned char *buf, unsigned int size, unsigned int timeout,
	   unsigned int *read, void *d)
{
	int r = 0;
	gp_port_set_timeout ((GPPort *) d, timeout);
	for (*read = 0; *read < size; (*read)++) {
		r = gp_port_read ((GPPort *) d, &buf[*read], 1);
		if (r < 0) break;
	}
	if ((*read == 0) && (r < 0)) return KNC_CNTRL_RES_ERR;
	return KNC_CNTRL_RES_OK;
}

static KncCntrlRes
write_func (const unsigned char *buf, unsigned int size, void *d)
{
	if (gp_port_write ((GPPort *) d, buf, size) < 0)
		return KNC_CNTRL_RES_ERR;
	return KNC_CNTRL_RES_OK;
}

static void
free_func (KncCntrl *c, void *d)
{
	gp_port_free ((GPPort *) d);
}

static void
knc_c_camera_init (KncCCamera *c)
{
	c->priv = g_new0 (KncCCameraPriv, 1);
}

KncCCamera *
knc_c_camera_new (const char *manufacturer, const char *model, GPPort *p,
		  CORBA_Environment *ev)
{
	KncCCamera *c = g_object_new (KNC_C_TYPE_CAMERA, NULL);
	GPPortSettings s;
	KncCamRes cam_res;
	KncCntrlRes cntrl_res;

	gp_port_get_settings (p, &s);
	s.serial.speed = 9600;
	s.serial.parity = GP_PORT_SERIAL_PARITY_OFF;
	gp_port_set_settings (p, s);

	c->priv->c = knc_cntrl_new (read_func, write_func, p);
	knc_cntrl_set_func_free (c->priv->c, free_func, p);
	c->priv->manufacturer = g_strdup (manufacturer);
	c->priv->model = g_strdup (model);

	cntrl_res = knc_reset_prefs (c->priv->c, &cam_res);
	if (cntrl_res || cam_res) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					ex_GNOME_C_Error, NULL);
		g_object_unref (c);
		return NULL;
	}

	return c;
}

BONOBO_TYPE_FUNC_FULL (KncCCamera, GNOME_C_Camera, BONOBO_TYPE_OBJECT, knc_c_camera);
