#include <config.h>
#include "gknc-preview.h"

#include <libknc/knc.h>

#include <gtk/gtkstock.h>

#include <gdk-pixbuf/gdk-pixbuf-loader.h>

struct _GkncPreviewPriv {
	GMutex *c_mutex;
	KncCntrl *c;
	GThread *t;
};

#define PARENT_TYPE GTK_TYPE_IMAGE
static GObjectClass *parent_class;
 
static void
gknc_preview_finalize (GObject *o)
{
	GkncPreview *p = GKNC_PREVIEW (o);

	gknc_preview_stop (p);
	g_mutex_free (p->priv->c_mutex);
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
		return KNC_CNTRL_ERR_CANCEL;
	}
	return KNC_CNTRL_OK;
}

void
gknc_preview_refresh (GkncPreview *p, KncCntrl *cntrl)
{
        KncCntrlRes cntrl_res;
        KncCamRes cam_res;
        GdkPixbufLoader *l = gdk_pixbuf_loader_new ();
	GdkPixbuf *pixbuf;
        GError *e = NULL;

	g_return_if_fail (GKNC_IS_PREVIEW (p));

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

static gpointer
thread_func (gpointer data)
{
        GkncPreview *p = GKNC_PREVIEW (data);

	while (1) {
		g_mutex_lock (p->priv->c_mutex);
		if (!p->priv->c) {
			g_mutex_unlock (p->priv->c_mutex);
			return NULL;
		}
		gknc_preview_refresh (p, p->priv->c);
		g_mutex_unlock (p->priv->c_mutex);
	}
}

void
gknc_preview_start (GkncPreview *p, KncCntrl *c)
{
	gknc_preview_stop (p);
	p->priv->c = c;
	knc_cntrl_ref (c);
	p->priv->t = g_thread_create (thread_func, p, TRUE, NULL);
}

void
gknc_preview_stop (GkncPreview *p)
{
	g_mutex_lock (p->priv->c_mutex);
	if (p->priv->c) {knc_cntrl_unref (p->priv->c); p->priv->c = NULL;}
	g_mutex_unlock (p->priv->c_mutex);

	if (p->priv->t) {
		g_thread_join (p->priv->t);
		p->priv->t = NULL;
	}
}

static void
gknc_preview_class_init (gpointer klass, gpointer class_data)
{
	GObjectClass *g_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (g_class);

	 g_class->finalize = gknc_preview_finalize;
}
                                                                                
static void
gknc_preview_init (GTypeInstance *instance, gpointer g_class)
{
	GkncPreview *p = GKNC_PREVIEW (instance);

	p->priv = g_new0 (GkncPreviewPriv, 1);
	p->priv->c_mutex = g_mutex_new ();
}

GType
gknc_preview_get_type (void)
{
	static GType t = 0;

	if (!t) {
	    GTypeInfo ti = {
		sizeof (GkncPreviewClass), NULL, NULL,
		gknc_preview_class_init, NULL, NULL,
		sizeof (GkncPreview), 0,
		gknc_preview_init, NULL};
	    t = g_type_register_static (PARENT_TYPE, "GkncPreview", &ti,
			    		0);
	}

	return t;
}

GkncPreview *
gknc_preview_new (void)
{
	GkncPreview *p = g_object_new (GKNC_TYPE_PREVIEW, NULL);

	gtk_image_set_from_stock (GTK_IMAGE (p), GTK_STOCK_DIALOG_QUESTION,
				  GTK_ICON_SIZE_DIALOG);

	return p;
}
