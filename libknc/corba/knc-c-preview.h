#ifndef __KNC_C_PREVIEW_H__
#define __KNC_C_PREVIEW_H__

#include <bonobo/bonobo-control.h>
#include <libknc/knc-cntrl.h>
#include <GNOME_C.h>

G_BEGIN_DECLS

#define KNC_C_TYPE_PREVIEW (knc_c_preview_get_type ())
#define KNC_C_PREVIEW(o) (G_TYPE_CHECK_INSTANCE_CAST((o),KNC_C_TYPE_PREVIEW,KncCPreview))
#define KNC_C_PREVIEW_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k),KNC_C_TYPE_PREVIEW,KncCPreviewClass))
#define KNC_C_IS_PREVIEW(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),KNC_C_TYPE_PREVIEW))

typedef struct _KncCPreview      KncCPreview;
typedef struct _KncCPreviewClass KncCPreviewClass;
typedef struct _KncCPreviewPriv  KncCPreviewPriv;

struct _KncCPreview {
	BonoboControl parent;

	KncCPreviewPriv *priv;
};

struct _KncCPreviewClass {
	BonoboControlClass parent_class;

	POA_GNOME_C_Preview__epv epv;
};

GType        knc_c_preview_get_type (void);
KncCPreview *knc_c_preview_new      (KncCntrl *);

G_END_DECLS

#endif
