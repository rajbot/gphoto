#include <config.h>
#include "knc-c-capture.h"
#include "knc-c-preview-widget.h"

#include <libknc/knc.h>

#include <glade/glade.h>

#include <gtk/gtktogglebutton.h>
#include <gtk/gtkbutton.h>

#define _(s) (s)
#define N_(s) (s)

struct _KncCCapturePriv {
	KncCntrl *c;
	GladeXML *xml;
};

static GObjectClass *parent_class;
                                                                                
static void
knc_c_capture_finalize (GObject *o)
{
	KncCCapture *c = KNC_C_CAPTURE (o);

	 g_object_unref (G_OBJECT (c->priv->xml));
	 knc_cntrl_unref (c->priv->c);
	 g_free (c->priv);

	 G_OBJECT_CLASS (parent_class)->finalize (o);
}
                                                                                
static void
knc_c_capture_class_init (KncCCaptureClass *klass)
{
	GObjectClass *g_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	g_class->finalize = knc_c_capture_finalize;
}

static void
knc_c_capture_init (KncCCapture *c)
{
	c->priv = g_new0 (KncCCapturePriv, 1);
}

#if 0
static gboolean
knc_c_capture_check_result (KncCCapture *c, KncCntrlRes cntrl_res,
			    KncCamRes cam_res)
{
	if (cntrl_res) {
		g_warning ("Operation failed: %s",
			   knc_cntrl_res_name (cntrl_res));
		return FALSE;
	}
	if (cam_res) {
		g_warning ("Operation failed: %s",
			   knc_cam_res_name (cam_res));
		return FALSE;
	}
	return TRUE;
}
#endif

static gboolean
on_refresh_clicked (GtkButton *button, KncCCapture *c)
{
	GtkWidget *w = glade_xml_get_widget (c->priv->xml, "custom_preview");

	knc_c_preview_widget_refresh (KNC_C_PREVIEW_WIDGET (w), c->priv->c);
	return TRUE;
}

static void
on_refresh_continuously_toggled (GtkToggleButton *t, KncCCapture *c)
{
	GtkWidget *w = glade_xml_get_widget (c->priv->xml, "custom_preview");

	if (t->active)
		knc_c_preview_widget_start (KNC_C_PREVIEW_WIDGET (w),
					    c->priv->c);
	else
		knc_c_preview_widget_stop (KNC_C_PREVIEW_WIDGET (w));
	g_assert ((w = glade_xml_get_widget (c->priv->xml, "button_refresh")));
	gtk_widget_set_sensitive (w, !t->active);
}

KncCCapture *
knc_c_capture_new (KncCntrl *cntrl)
{
	KncCCapture *c = g_object_new (KNC_C_TYPE_CAPTURE, NULL);
	GtkWidget *w;
#if 0
	KncCamRes cam_res;
	KncStatus s;
	KncPrefs prefs;
	KncCntrlRes cntrl_res;
#endif

	c->priv->c = cntrl;
	knc_cntrl_ref (cntrl);

	/* Read the interface description */
	c->priv->xml = glade_xml_new (KNC_GLADE_DIR "/knc.glade",
				      "table_capture", NULL);
	if (!c->priv->xml) c->priv->xml = glade_xml_new (
		KNC_SRC_DIR "/corba/knc.glade", "table_capture", NULL);
	g_assert (c->priv->xml);
	g_assert ((w = glade_xml_get_widget (c->priv->xml, "table_capture")));
	bonobo_control_construct (BONOBO_CONTROL (c), w);

	/* Load the current settings */
	g_warning ("Fixme!");

	/* Connect the signals */
	g_assert ((w = glade_xml_get_widget (c->priv->xml,
					"checkbutton_refresh_continuously")));
	g_signal_connect (w, "toggled",
			  G_CALLBACK (on_refresh_continuously_toggled), c);
	g_assert ((w = glade_xml_get_widget (c->priv->xml, "button_refresh")));
	g_signal_connect (w, "clicked", G_CALLBACK (on_refresh_clicked), c);

	return c;
}

BONOBO_TYPE_FUNC (KncCCapture, BONOBO_TYPE_CONTROL, knc_c_capture);
