#include <config.h>
#include "knc-c-file.h"
#include "knc-c-if.h"

#include <bonobo/bonobo-exception.h>

#include <libknc/knc.h>

struct _KncCFilePriv {
	KncCntrl *c;
	unsigned long id;
};

static GObjectClass *parent_class;

static void
knc_c_file_finalize (GObject *o)
{
	KncCFile *f = KNC_C_FILE (o);

	knc_cntrl_unref (f->priv->c);
	g_free (f->priv);

	G_OBJECT_CLASS (parent_class)->finalize (o);
}

static GNOME_C_IDList *
impl_get_ifs (PortableServer_Servant servant, CORBA_Environment *ev)
{
	GNOME_C_IDList *l = GNOME_C_IDList__alloc ();
	unsigned int i;

	l->_length = 3;
	l->_buffer = CORBA_sequence_CORBA_unsigned_long_allocbuf (l->_length);
	for (i = 0; i < l->_length; i++) l->_buffer[i] = i;
	CORBA_sequence_set_release (l, CORBA_TRUE);
	return l;
}

static GNOME_C_If
impl_get_if (PortableServer_Servant servant, GNOME_C_ID id,
	     CORBA_Environment *ev)
{
	KncCFile *f = KNC_C_FILE (bonobo_object (servant));
	KncCIf *i;
	KncImage im = 0;

	switch (id) {
	case 0: im = KNC_IMAGE_EXIF; break;
	case 1: im = KNC_IMAGE_THUMB; break;
	case 2: im = KNC_IMAGE_JPEG; break;
	}
	i = knc_c_if_new (f->priv->c, 0, im, ev);
	if (BONOBO_EX (ev)) return CORBA_OBJECT_NIL;
	return CORBA_Object_duplicate (BONOBO_OBJREF (i), ev);
}

static void
knc_c_file_class_init (KncCFileClass *klass)
{
	POA_GNOME_C_File__epv *epv = &klass->epv;
	GObjectClass *g_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	g_class->finalize = knc_c_file_finalize;

	epv->get_ifs = impl_get_ifs;
	epv->get_if = impl_get_if;
}

static void
knc_c_file_init (KncCFile *f)
{
	f->priv = g_new0 (KncCFilePriv, 1);
}

KncCFile *
knc_c_file_new (KncCntrl *c, unsigned long n, CORBA_Environment *ev)
{
	KncCFile *f = g_object_new (KNC_C_TYPE_FILE, NULL);

	f->priv->c = c;
	knc_cntrl_ref (c);
	f->priv->id = n;
	return f;
}

BONOBO_TYPE_FUNC_FULL (KncCFile, GNOME_C_File, BONOBO_TYPE_OBJECT, knc_c_file);
