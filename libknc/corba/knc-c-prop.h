#ifndef __KNC_C_PROP_H__
#define __KNC_C_PROP_H__

#include <bonobo/bonobo-object.h>
#include <GNOME_C.h>
#include <libknc/knc-cntrl.h>

G_BEGIN_DECLS

#define KNC_C_TYPE_PROP (knc_c_prop_get_type ())
#define KNC_C_PROP(o) (G_TYPE_CHECK_INSTANCE_CAST((o),KNC_C_TYPE_PROP,KncCProp))
#define KNC_C_PROP_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k),KNC_C_TYPE_PROP,KncCPropClass))
#define KNC_C_IS_PROP(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),KNC_C_TYPE_PROP))

typedef struct _KncCProp      KncCProp;
typedef struct _KncCPropClass KncCPropClass;
typedef struct _KncCPropPriv  KncCPropPriv;

struct _KncCProp {
	BonoboObject parent;

	KncCPropPriv *priv;
};

struct _KncCPropClass {
	BonoboObjectClass parent_class;

	POA_GNOME_C_Prop__epv epv;
};

typedef enum {
	KNC_C_PROP_TYPE_TV_OUTPUT_FORMAT,
	KNC_C_PROP_TYPE_DATE_FORMAT,
	KNC_C_PROP_TYPE_LANGUAGE
} KncCPropType;

GType    knc_c_prop_get_type (void);
KncCProp *knc_c_prop_new      (KncCntrl *, KncCPropType);

G_END_DECLS

#endif
