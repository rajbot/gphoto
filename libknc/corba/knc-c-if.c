#include <config.h>
#include "knc-c-if.h"

#include <string.h>

#include <bonobo/bonobo-exception.h>

struct _KncCIfPriv {
	KncCntrl *c;
	KncImage i;
	unsigned long n;
};

static GObjectClass *parent_class;

static void
knc_c_if_finalize (GObject *o)
{
	KncCIf *i = KNC_C_IF (o);

	knc_cntrl_unref (i->priv->c);
	g_free (i->priv);

	G_OBJECT_CLASS (parent_class)->finalize (o);
}

static CORBA_string
impl_get_name (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCIf *i = KNC_C_IF (bonobo_object (servant));

	switch (i->priv->i) {
	case KNC_IMAGE_THUMB:
		return CORBA_string_dup ("Preview");
	case KNC_IMAGE_JPEG:
		return CORBA_string_dup ("File (short)");
	case KNC_IMAGE_EXIF:
		return CORBA_string_dup ("File");
	default:
		return NULL;
	}
}

static KncCntrlRes
func_data (const unsigned char *buf, unsigned int size, void *d)
{
	GNOME_C_ReadCallback cb = d;
	CORBA_sequence_CORBA_octet *b = CORBA_sequence_CORBA_octet__alloc ();
	CORBA_Environment ev;
	KncCntrlRes r;

	b->_length = size;
	b->_buffer = CORBA_sequence_CORBA_octet_allocbuf (b->_length);
	memcpy (b->_buffer, buf, size);

	CORBA_exception_init (&ev);
	GNOME_C_ReadCallback_read (cb, b, &ev);
	r = BONOBO_EX (&ev) ? KNC_CNTRL_ERR_CANCEL : KNC_CNTRL_OK;
	CORBA_exception_free (&ev);

	return r;
}

static void
impl_read (PortableServer_Servant servant, GNOME_C_ReadCallback cb,
	   CORBA_Environment *ev)
{
	KncCIf *i = KNC_C_IF (bonobo_object (servant));
	KncCamRes cam_res;
	KncCntrlRes cntrl_res;

	knc_cntrl_set_func_data (i->priv->c, func_data, cb);
	cntrl_res = knc_get_image (i->priv->c, &cam_res, i->priv->n,
				   KNC_SOURCE_CARD, i->priv->i);
	knc_cntrl_set_func_data (i->priv->c, NULL, NULL);
	if (cntrl_res || cam_res)
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_GNOME_C_Error, NULL);
}

static void
knc_c_if_class_init (KncCIfClass *klass)
{
	POA_GNOME_C_If__epv *epv = &klass->epv;
	GObjectClass *g_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	epv->read = impl_read;
	epv->_get_name = impl_get_name;

	g_class->finalize = knc_c_if_finalize;
}

static void
knc_c_if_init (KncCIf *i)
{
	i->priv = g_new0 (KncCIfPriv, 1);
}

KncCIf *
knc_c_if_new (KncCntrl *c, unsigned long n, KncImage im, CORBA_Environment *ev)
{
	KncCIf *i = g_object_new (KNC_C_TYPE_IF, NULL);

	i->priv->n = n;
	i->priv->i = im;

	return i;
}

BONOBO_TYPE_FUNC_FULL (KncCIf, GNOME_C_If, BONOBO_TYPE_OBJECT, knc_c_if);
