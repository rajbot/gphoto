#include <config.h>
#include "knc-c-bag.h"

#define _(s) (s)

#include <stdio.h>
#include <string.h>

struct _KncCBagPriv {
	KncCntrl *c;
	KncCBagType t;
};

static GObjectClass *parent_class;

static void
knc_c_bag_finalize (GObject *o)
{
	KncCBag *b = KNC_C_BAG (o);

	knc_cntrl_unref (b->priv->c);
	g_free (b->priv);

	G_OBJECT_CLASS (parent_class)->finalize (o);
}

static CORBA_string
impl_get_name (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCBag *b = KNC_C_BAG (bonobo_object (servant));

	switch (b->priv->t) {
	case KNC_C_BAG_TYPE_LOCALIZATION:
		return _("Localization");
	default:
		CORBA_exception_set (ev, CORBA_SYSTEM_EXCEPTION, 
				     ex_CORBA_UNKNOWN, NULL);
		return NULL;
	}
}

static CORBA_string
impl_get_description (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCBag *b = KNC_C_BAG (bonobo_object (servant));

	 switch (b->priv->t) {
	 case KNC_C_BAG_TYPE_LOCALIZATION:
		 return _("Change the language, date format, etc.");
	default:
		 CORBA_exception_set (ev, CORBA_SYSTEM_EXCEPTION,
				      ex_CORBA_UNKNOWN, NULL);
		 return NULL;
	 }
}

static GNOME_C_Icon *
impl_get_icon (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCBag *b = KNC_C_BAG (bonobo_object (servant));
	GNOME_C_Icon *icon;
	FILE *f;
	guint i;
	size_t n;
	gchar buf[1024];

	switch (b->priv->t) {
	case KNC_C_BAG_TYPE_LOCALIZATION:
		f = fopen (ICONSDIR "knc-globe.png", "r");
		if (!f) {
			CORBA_exception_set (ev, CORBA_SYSTEM_EXCEPTION,
					     ex_CORBA_UNKNOWN, NULL);
			return NULL;
		}
		fseek (f, 0,  SEEK_END);
		icon = GNOME_C_Icon__alloc ();
		icon->_length = ftell (f);
		icon->_buffer = GNOME_C_Icon_allocbuf (icon->_length);
		CORBA_sequence_set_release (icon, TRUE);
		for (i = 0; i < icon->_length; i+= 1024) {
			n = fread (buf, sizeof (gchar), sizeof (buf), f);
			memcpy (icon->_buffer + i, buf, n);
		}
		return icon;
	default:
		CORBA_exception_set (ev, CORBA_SYSTEM_EXCEPTION,
				     ex_CORBA_UNKNOWN, NULL);
		return NULL;
	}
}

static void
knc_c_bag_class_init (KncCBagClass *klass)
{
	POA_GNOME_C_Bag__epv *epv = &klass->epv;
	GObjectClass *g_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	epv->_get_name = impl_get_name;
	epv->_get_description = impl_get_description;
	epv->_get_icon = impl_get_icon;

	g_class->finalize = knc_c_bag_finalize;
}

static void
knc_c_bag_init (KncCBag *b)
{
	b->priv = g_new0 (KncCBagPriv, 1);
}

KncCBag *
knc_c_bag_new (KncCntrl *c, KncCBagType t)
{
	KncCBag *b = g_object_new (KNC_C_TYPE_BAG, NULL);

	b->priv->c = c;
	knc_cntrl_ref (c);

	b->priv->t = t;

	return b;
}

BONOBO_TYPE_FUNC_FULL (KncCBag, GNOME_C_Bag, BONOBO_TYPE_OBJECT, knc_c_bag);
