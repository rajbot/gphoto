#ifndef __KNC_C_BAG_H__
#define __KNC_C_BAG_H__

#include <bonobo/bonobo-object.h>
#include <GNOME_C.h>
#include <libknc/knc-cntrl.h>

G_BEGIN_DECLS

#define KNC_C_TYPE_BAG (knc_c_bag_get_type ())
#define KNC_C_BAG(o) (G_TYPE_CHECK_INSTANCE_CAST((o),KNC_C_TYPE_BAG,KncCBag))
#define KNC_C_BAG_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k),KNC_C_TYPE_BAG,KncCBagClass))
#define KNC_C_IS_BAG(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),KNC_C_TYPE_BAG))

typedef struct _KncCBag      KncCBag;
typedef struct _KncCBagClass KncCBagClass;
typedef struct _KncCBagPriv  KncCBagPriv;

struct _KncCBag {
	BonoboObject parent;

	KncCBagPriv *priv;
};

struct _KncCBagClass {
	BonoboObjectClass parent_class;

	POA_GNOME_C_Bag__epv epv;
};

typedef enum {
	KNC_C_BAG_TYPE_ROOT,
	KNC_C_BAG_TYPE_LOCALIZATION
} KncCBagType;

GType    knc_c_bag_get_type (void);
KncCBag *knc_c_bag_new      (KncCntrl *, KncCBagType);

G_END_DECLS

#endif
