#include <config.h>
#include "knc-c-preview-widget.h"

#include <libknc/knc.h>

#include <gdk-pixbuf/gdk-pixbuf-loader.h>

struct _KncCPreviewWidgetPriv {
	KncCntrl *c;
	guint id;
};

#define PARENT_TYPE GTK_TYPE_IMAGE
static GObjectClass *parent_class;
 
static void
knc_c_preview_widget_finalize (GObject *o)
{
	KncCPreviewWidget *p = KNC_C_PREVIEW_WIDGET (o);

	knc_c_preview_widget_stop (p);
	g_free (p->priv);

	G_OBJECT_CLASS (parent_class)->finalize (o);
}
 
static KncCntrlRes
func_data (const unsigned char *data, unsigned int size, void *d)
{
	GdkPixbufLoader *l = GDK_PIXBUF_LOADER (d);
	GError *e = NULL;

	gdk_pixbuf_loader_write (l, data, size, &e);
	if (e) {
		g_warning ("Could not interpret data: %s", e->message);
		g_error_free (e);
		return KNC_CNTRL_RES_ERR_CANCEL;
	}
	return KNC_CNTRL_RES_OK;
}

void
knc_c_preview_widget_refresh (KncCPreviewWidget *p, KncCntrl *cntrl)
{
        KncCntrlRes cntrl_res;
        KncCamRes cam_res;
        GdkPixbufLoader *l = gdk_pixbuf_loader_new ();
	GdkPixbuf *pixbuf;
        GError *e = NULL;
 
        knc_cntrl_set_func_data (cntrl, func_data, l);
        cntrl_res = knc_get_preview (cntrl, &cam_res, KNC_PREVIEW_YES);
        knc_cntrl_set_func_data (cntrl, NULL, NULL);
        if (cntrl_res || cam_res) {
		gdk_pixbuf_loader_close (l, NULL);
                g_object_unref (G_OBJECT (l));
                g_warning ("Could not capture preview: '%s'/'%s'",
                           knc_cntrl_res_name (cntrl_res),
                           knc_cam_res_name (cam_res));
                return;
        }
        gdk_pixbuf_loader_close (l, &e);
        if (e) {
                g_object_unref (G_OBJECT (l));
                g_warning ("Could not interpret data: %s", e->message);
                g_error_free (e);
                return;
        }
	g_assert ((pixbuf = gdk_pixbuf_loader_get_pixbuf (l)));
        gtk_image_set_from_pixbuf (GTK_IMAGE (p), pixbuf);
        g_object_unref (G_OBJECT (l));
}

static gboolean
func_idle_preview (gpointer data)
{
        KncCPreviewWidget *p = KNC_C_PREVIEW_WIDGET (data);

        knc_c_preview_widget_refresh (p, p->priv->c);
        return TRUE;
}

void
knc_c_preview_widget_start (KncCPreviewWidget *p, KncCntrl *c)
{
	knc_c_preview_widget_stop (p);
	p->priv->c = c;
	knc_cntrl_ref (c);
	p->priv->id = g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
					    func_idle_preview, p, NULL);
}

void
knc_c_preview_widget_stop (KncCPreviewWidget *p)
{
	if (p->priv->id) {
		g_source_remove (p->priv->id);
		p->priv->id = 0;
	}
	if (p->priv->c) {knc_cntrl_unref (p->priv->c); p->priv->c = NULL;}
}

static void
knc_c_preview_widget_class_init (gpointer klass, gpointer class_data)
{
	GObjectClass *g_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (g_class);

	 g_class->finalize = knc_c_preview_widget_finalize;
}
                                                                                
static void
knc_c_preview_widget_init (GTypeInstance *instance, gpointer g_class)
{
	KncCPreviewWidget *p = KNC_C_PREVIEW_WIDGET (instance);

	p->priv = g_new0 (KncCPreviewWidgetPriv, 1);
}

GType
knc_c_preview_widget_get_type (void)
{
	static GType t = 0;

	if (!t) {
	    GTypeInfo ti = {
		sizeof (KncCPreviewWidgetClass), NULL, NULL,
		knc_c_preview_widget_class_init, NULL, NULL,
		sizeof (KncCPreviewWidget), 0,
		knc_c_preview_widget_init, NULL};
	    t = g_type_register_static (PARENT_TYPE, "KncCPreviewWidget", &ti,
			    		0);
	}

	return t;
}

KncCPreviewWidget *
knc_c_preview_widget_new (void)
{
	KncCPreviewWidget *p = g_object_new (KNC_C_TYPE_PREVIEW_WIDGET, NULL);

	return p;
}
