#include <config.h>
#include "knc-c-prop.h"

#define _(s) (s)
#define N_(s) (s)

struct _KncCPropPriv {
	KncCntrl *c;
	KncCPropType t;
};

static GObjectClass *parent_class;

static void
knc_c_prop_finalize (GObject *o)
{
	KncCProp *p = KNC_C_PROP (o);

	knc_cntrl_unref (p->priv->c);
	g_free (p->priv);

	G_OBJECT_CLASS (parent_class)->finalize (o);
}

static struct {
	KncCPropType t;
	const char *name;
	const char *descr;
} PropNames[] = {
	{KNC_C_PROP_TYPE_LANGUAGE, N_("Language"),
		N_("Language of the text displayed on your camera")},
	{KNC_C_PROP_TYPE_TV_OUTPUT_FORMAT, N_("TV Output Format"),
		N_("Format of the TV signal")},
	{KNC_C_PROP_TYPE_DATE_FORMAT, N_("Date Format"),
		N_("Format of the date")},
	{0, NULL}
};

static CORBA_string
impl_get_name (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCProp *p = KNC_C_PROP (bonobo_object (servant));
	guint i;

	for (i = 0; PropNames[i].name; i++)
		if (PropNames[i].t == p->priv->t) break;
	return PropNames[i].name ?
			CORBA_string_dup (_(PropNames[i].name)) : NULL;
}

static CORBA_string
impl_get_description (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCProp *p = KNC_C_PROP (bonobo_object (servant));
	guint i;

	for (i = 0; PropNames[i].descr; i++)
		if (PropNames[i].t == p->priv->t) break;
	return PropNames[i].descr ? 
			CORBA_string_dup (_(PropNames[i].descr)) : NULL;
}

static GNOME_C_Val *
impl_get_val (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCProp *p = KNC_C_PROP (bonobo_object (servant));
	GNOME_C_Val *val = GNOME_C_Val__alloc ();

	switch (p->priv->t) {
	case KNC_C_PROP_TYPE_LANGUAGE:
	    val->_d = GNOME_C_VAL_TYPE_CHOICE;
	    val->_u.p_choice.val = 0;
	    val->_u.p_choice.vals._length = 0;
	    break;
	case KNC_C_PROP_TYPE_DATE_FORMAT:
	    val->_d = GNOME_C_VAL_TYPE_CHOICE;
	    val->_u.p_choice.val = 3;
	    val->_u.p_choice.vals._length = 3;
	    val->_u.p_choice.vals._buffer =
		    CORBA_sequence_CORBA_string_allocbuf (3);
	    val->_u.p_choice.vals._buffer[0] =
		    CORBA_string_dup (_("Month/Day/Year"));
	    val->_u.p_choice.vals._buffer[1] = 
		    CORBA_string_dup (_("Day/Month/Year"));
	    val->_u.p_choice.vals._buffer[2] = 
		    CORBA_string_dup (_("Year/Month/Day"));
	    break;
	case KNC_C_PROP_TYPE_TV_OUTPUT_FORMAT:
	    val->_d = GNOME_C_VAL_TYPE_CHOICE;
	    val->_u.p_choice.val = 3;
	    val->_u.p_choice.vals._length = 3;
	    val->_u.p_choice.vals._buffer =
		    CORBA_sequence_CORBA_string_allocbuf (3);
	    val->_u.p_choice.vals._buffer[0] = CORBA_string_dup (_("NTSC"));
	    val->_u.p_choice.vals._buffer[1] = CORBA_string_dup (_("PAL"));
	    val->_u.p_choice.vals._buffer[2] = CORBA_string_dup (_("Hide"));
	    break;
	}

	return val;
}

static void
knc_c_prop_class_init (KncCPropClass *klass)
{
	POA_GNOME_C_Prop__epv *epv = &klass->epv;
	GObjectClass *g_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	epv->_get_name = impl_get_name;
	epv->_get_description = impl_get_description;
	epv->_get_val = impl_get_val;

	g_class->finalize = knc_c_prop_finalize;
}

static void
knc_c_prop_init (KncCProp *p)
{
	p->priv = g_new0 (KncCPropPriv, 1);
}

KncCProp *
knc_c_prop_new (KncCntrl *c, KncCPropType t)
{
	KncCProp *p = g_object_new (KNC_C_TYPE_PROP, NULL);

	p->priv->c = c;
	knc_cntrl_ref (c);

	p->priv->t = t;

	return p;
}

BONOBO_TYPE_FUNC_FULL (KncCProp, GNOME_C_Prop, BONOBO_TYPE_OBJECT, knc_c_prop);
