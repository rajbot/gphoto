#ifndef __KNC_C_PREVIEW_WIDGET_H__
#define __KNC_C_PREVIEW_WIDGET_H__

#include <gtk/gtkimage.h>
#include <libknc/knc-cntrl.h>

G_BEGIN_DECLS

#define KNC_C_TYPE_PREVIEW_WIDGET (knc_c_preview_widget_get_type ())
#define KNC_C_PREVIEW_WIDGET(o) (G_TYPE_CHECK_INSTANCE_CAST((o),KNC_C_TYPE_PREVIEW_WIDGET,KncCPreviewWidget))
#define KNC_C_PREVIEW_WIDGET_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k),KNC_C_TYPE_PREVIEW_WIDGET,KncCPreviewWidgetClass))
#define KNC_C_IS_PREVIEW_WIDGET(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),KNC_C_TYPE_PREVIEW_WIDGET))

typedef struct _KncCPreviewWidget      KncCPreviewWidget;
typedef struct _KncCPreviewWidgetClass KncCPreviewWidgetClass;
typedef struct _KncCPreviewWidgetPriv  KncCPreviewWidgetPriv;

struct _KncCPreviewWidget {
	GtkImage parent;

	KncCPreviewWidgetPriv *priv;
};

struct _KncCPreviewWidgetClass {
	GtkImageClass parent_class;
};

GType knc_c_preview_widget_get_type (void);

KncCPreviewWidget *knc_c_preview_widget_new (void);

void  knc_c_preview_widget_start   (KncCPreviewWidget *, KncCntrl *);
void  knc_c_preview_widget_stop    (KncCPreviewWidget *);
void  knc_c_preview_widget_refresh (KncCPreviewWidget *, KncCntrl *);

G_END_DECLS

#endif
