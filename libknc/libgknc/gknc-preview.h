#ifndef __GKNC_PREVIEW_H__
#define __GKNC_PREVIEW_H__

#include <gtk/gtkimage.h>
#include <libknc/knc-cntrl.h>

G_BEGIN_DECLS

#define GKNC_TYPE_PREVIEW (gknc_preview_get_type ())
#define GKNC_PREVIEW(o) (G_TYPE_CHECK_INSTANCE_CAST((o),GKNC_TYPE_PREVIEW,GkncPreview))
#define GKNC_PREVIEW_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k),GKNC_TYPE_PREVIEW,GkncPreviewClass))
#define GKNC_IS_PREVIEW(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),GKNC_TYPE_PREVIEW))

typedef struct _GkncPreview      GkncPreview;
typedef struct _GkncPreviewClass GkncPreviewClass;
typedef struct _GkncPreviewPriv  GkncPreviewPriv;

struct _GkncPreview {
	GtkImage parent;

	GkncPreviewPriv *priv;
};

struct _GkncPreviewClass {
	GtkImageClass parent_class;
};

GType gknc_preview_get_type (void);

GkncPreview *gknc_preview_new (void);

void  gknc_preview_start   (GkncPreview *, KncCntrl *);
void  gknc_preview_stop    (GkncPreview *);
void  gknc_preview_refresh (GkncPreview *, KncCntrl *);

G_END_DECLS

#endif
