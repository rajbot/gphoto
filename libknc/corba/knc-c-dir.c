#include <config.h>
#include "knc-c-dir.h"
#include "knc-c-file.h"

#include <libknc/knc.h>

#include <bonobo/bonobo-exception.h>

#define C0(cntrl_res,cam_res,ev) {				\
	if (cntrl_res || cam_res) {				\
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,	\
			ex_GNOME_C_Error, NULL);		\
		return 0;					\
	}							\
}
#define CNIL(cntrl_res,cam_res,ev) {				\
	if (cntrl_res || cam_res) {				\
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,	\
			ex_GNOME_C_Error, NULL);		\
		return CORBA_OBJECT_NIL;			\
	}							\
}

struct _KncCDirPriv {
	KncCntrl *c;
};

static GObjectClass *parent_class;

static void
knc_c_dir_finalize (GObject *o)
{
	KncCDir *d = KNC_C_DIR (o);

	knc_cntrl_unref (d->priv->c);
	g_free (d->priv);

	G_OBJECT_CLASS (parent_class)->finalize (o);
}

static GNOME_C_IDList *
impl_get_dirs (PortableServer_Servant servant, CORBA_Environment *ev)
{
	GNOME_C_IDList *l = GNOME_C_IDList__alloc ();

	l->_length = 0;
	CORBA_sequence_set_release (l, CORBA_TRUE);
	return l;
}

static GNOME_C_IDList *
impl_get_files (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCDir *d = KNC_C_DIR (bonobo_object (servant));
	KncStatus s;
	KncCamRes cam_res;
	KncCntrlRes cntrl_res;
	unsigned int i;
	KncImageInfo info;
	GNOME_C_IDList *l;

	cntrl_res = knc_get_status (d->priv->c, &cam_res, &s);
	CNIL (cntrl_res, cam_res, ev);
	l = GNOME_C_IDList__alloc ();
	l->_length = 0;
	l->_buffer = CORBA_sequence_CORBA_unsigned_long_allocbuf (s.pictures);
	for (i = 0; i < s.pictures; i++) {
		cntrl_res = knc_get_image_info (d->priv->c, &cam_res, i, &info);
		if (cntrl_res || cam_res)
			continue;
		l->_buffer[l->_length] = info.id;
		l->_length++;
	}
	CORBA_sequence_set_release (l, CORBA_TRUE);
	return l;
}

static GNOME_C_File
impl_get_file (PortableServer_Servant servant, CORBA_unsigned_long id,
	       CORBA_Environment *ev)
{
	KncCDir *d = KNC_C_DIR (bonobo_object (servant));
	KncCFile *f = knc_c_file_new (d->priv->c, id, ev);
	if (BONOBO_EX (ev)) return CORBA_OBJECT_NIL;
	return CORBA_Object_duplicate (BONOBO_OBJREF (f), ev);
}

static void
knc_c_dir_class_init (KncCDirClass *klass)
{
	POA_GNOME_C_Dir__epv *epv = &klass->epv;
	GObjectClass *g_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	epv->get_files = impl_get_files;
	epv->get_dirs  = impl_get_dirs;
	epv->get_file  = impl_get_file;

	g_class->finalize = knc_c_dir_finalize;
}

static void
knc_c_dir_init (KncCDir *d)
{
	d->priv = g_new0 (KncCDirPriv, 1);
}

KncCDir *
knc_c_dir_new (KncCntrl *c)
{
	KncCDir *d = g_object_new (KNC_C_TYPE_DIR, NULL);
	d->priv->c = c;
	knc_cntrl_ref (c);

	return d;
}

BONOBO_TYPE_FUNC_FULL (KncCDir, GNOME_C_Dir, BONOBO_TYPE_OBJECT, knc_c_dir);
