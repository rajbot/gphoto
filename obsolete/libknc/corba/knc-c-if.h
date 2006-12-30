#ifndef __KNC_C_IF_H__
#define __KNC_C_IF_H__

#include <bonobo/bonobo-object.h>
#include <GNOME_C.h>
#include <libknc/knc.h>

G_BEGIN_DECLS

#define KNC_C_TYPE_IF (knc_c_if_get_type ())
#define KNC_C_IF(o) (G_TYPE_CHECK_INSTANCE_CAST((o),KNC_C_TYPE_IF,KncCIf))
#define KNC_C_IF_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k),KNC_C_TYPE_IF,KncCIfClass))
#define KNC_C_IS_IF(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),KNC_C_TYPE_IF))

typedef struct _KncCIf      KncCIf;
typedef struct _KncCIfClass KncCIfClass;
typedef struct _KncCIfPriv  KncCIfPriv;

struct _KncCIf {
	BonoboObject parent;

	KncCIfPriv *priv;
};

struct _KncCIfClass {
	BonoboObjectClass parent_class;

	POA_GNOME_C_If__epv epv;
};

GType   knc_c_if_get_type (void);
KncCIf *knc_c_if_new      (KncCntrl *, unsigned long, KncImage,
			   CORBA_Environment *);

G_END_DECLS

#endif
