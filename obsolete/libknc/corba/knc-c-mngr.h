#ifndef __KNC_C_MNGR_H__
#define __KNC_C_MNGR_H__

#include <bonobo/bonobo-object.h>
#include <GNOME_C.h>

G_BEGIN_DECLS

#define KNC_C_TYPE_MNGR (knc_c_mngr_get_type ())
#define KNC_C_MNGR(o) (G_TYPE_CHECK_INSTANCE_CAST((o),KNC_C_TYPE_MNGR,KncCMngr))
#define KNC_C_MNGR_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k),KNC_C_TYPE_MNGR,KncCMngrClass))
#define KNC_C_IS_MNGR(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),KNC_C_TYPE_MNGR))

typedef struct _KncCMngr      KncCMngr;
typedef struct _KncCMngrClass KncCMngrClass;
typedef struct _KncCMngrPriv  KncCMngrPriv;

struct _KncCMngr {
	BonoboObject parent;

	KncCMngrPriv *priv;
};

struct _KncCMngrClass {
	BonoboObjectClass parent_class;

	POA_GNOME_C_Mngr__epv epv;
};

GType knc_c_mngr_get_type (void);

G_END_DECLS

#endif
