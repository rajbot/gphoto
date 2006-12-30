#include <config.h>
#include "knc-c-preview.h"

#include <libgknc/gknc-preview.h>

#include <libknc/knc.h>

#include <gtk/gtkimage.h>

#include <gdk-pixbuf/gdk-pixbuf-loader.h>

struct _KncCPreviewPriv {
	KncCntrl *c;
	GkncPreview *p;
};

static GObjectClass *parent_class;

static void
knc_c_preview_finalize (GObject *o)
{
	KncCPreview *p = KNC_C_PREVIEW (o);

	knc_cntrl_unref (p->priv->c);
	g_free (p->priv);

	G_OBJECT_CLASS (parent_class)->finalize (o);
}

static void
impl_refresh (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCPreview *p = KNC_C_PREVIEW (bonobo_object (servant));

	gknc_preview_refresh (p->priv->p, p->priv->c);
}

static void
impl_start (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCPreview *p = KNC_C_PREVIEW (bonobo_object (servant));

	gknc_preview_start (p->priv->p, p->priv->c);
}

static void
impl_stop (PortableServer_Servant servant, CORBA_Environment *ev)
{
	KncCPreview *p = KNC_C_PREVIEW (bonobo_object (servant));

	gknc_preview_stop (p->priv->p);
}

static void
knc_c_preview_class_init (KncCPreviewClass *klass)
{
	POA_GNOME_C_Preview__epv *epv = &klass->epv;
	GObjectClass *g_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	epv->refresh = impl_refresh;
	epv->start   = impl_start;
	epv->stop    = impl_stop;

	g_class->finalize = knc_c_preview_finalize;
}
                                                                                
static void
knc_c_preview_init (KncCPreview *p)
{
	p->priv = g_new0 (KncCPreviewPriv, 1);
}

KncCPreview *
knc_c_preview_new (KncCntrl *c)
{
	KncCPreview *p = g_object_new (KNC_C_TYPE_PREVIEW, NULL);
	GtkWidget *w = g_object_new (GKNC_TYPE_PREVIEW, NULL);

	p->priv->c = c;
	knc_cntrl_ref (c);
	p->priv->p = GKNC_PREVIEW (w);

	gtk_widget_show (w);
	bonobo_control_construct (BONOBO_CONTROL (p), w);

	return p;
}

BONOBO_TYPE_FUNC_FULL (KncCPreview, GNOME_C_Preview, BONOBO_TYPE_CONTROL, 
		       knc_c_preview);
