#ifndef __KNC_C_DIR_H__
#define __KNC_C_DIR_H__

#include <bonobo/bonobo-object.h>
#include <GNOME_C.h>
#include <libknc/knc-cntrl.h>

G_BEGIN_DECLS

#define KNC_C_TYPE_DIR (knc_c_dir_get_type ())
#define KNC_C_DIR(o) (G_TYPE_CHECK_INSTANCE_CAST((o),KNC_C_TYPE_DIR,KncCDir))
#define KNC_C_DIR_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k),KNC_C_TYPE_DIR,KncCDirClass))
#define KNC_C_IS_DIR(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),KNC_C_TYPE_DIR))

typedef struct _KncCDir      KncCDir;
typedef struct _KncCDirClass KncCDirClass;
typedef struct _KncCDirPriv  KncCDirPriv;

struct _KncCDir {
	BonoboObject parent;

	KncCDirPriv *priv;
};

struct _KncCDirClass {
	BonoboObjectClass parent_class;

	POA_GNOME_C_Dir__epv epv;
};

GType    knc_c_dir_get_type (void);
KncCDir *knc_c_dir_new      (KncCntrl *);

G_END_DECLS

#endif
