#ifndef __KNC_C_CAPTURE_H__
#define __KNC_C_CAPTURE_H__

#include <bonobo/bonobo-control.h>
#include <libknc/knc-cntrl.h>

G_BEGIN_DECLS

#define KNC_C_TYPE_CAPTURE (knc_c_capture_get_type ())
#define KNC_C_CAPTURE(o) (G_TYPE_CHECK_INSTANCE_CAST((o),KNC_C_TYPE_CAPTURE,KncCCapture))
#define KNC_C_CAPTURE_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k),KNC_C_TYPE_CAPTURE,KncCCaptureClass))
#define KNC_C_IS_CAPTURE(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),KNC_C_TYPE_CAPTURE))

typedef struct _KncCCapture      KncCCapture;
typedef struct _KncCCaptureClass KncCCaptureClass;
typedef struct _KncCCapturePriv  KncCCapturePriv;

struct _KncCCapture {
	BonoboControl parent;

	KncCCapturePriv *priv;
};

struct _KncCCaptureClass {
	BonoboControlClass parent_class;
};

GType        knc_c_capture_get_type (void);
KncCCapture *knc_c_capture_new      (KncCntrl *);

G_END_DECLS

#endif
