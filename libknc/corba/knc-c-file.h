#ifndef __KNC_C_FILE_H__
#define __KNC_C_FILE_H__

#include <bonobo/bonobo-object.h>
#include <GNOME_C.h>

#include <libknc/knc-cntrl.h>

G_BEGIN_DECLS

#define KNC_C_TYPE_FILE (knc_c_file_get_type ())
#define KNC_C_FILE(o) (G_TYPE_CHECK_INSTANCE_CAST((o),KNC_C_TYPE_FILE,KncCFile))
#define KNC_C_FILE_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k),KNC_C_TYPE_FILE,KncCFileClass))
#define KNC_C_IS_FILE(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),KNC_C_TYPE_FILE))

typedef struct _KncCFile      KncCFile;
typedef struct _KncCFileClass KncCFileClass;
typedef struct _KncCFilePriv  KncCFilePriv;

struct _KncCFile {
	BonoboObject parent;

	KncCFilePriv *priv;
};

struct _KncCFileClass {
	BonoboObjectClass parent_class;

	POA_GNOME_C_File__epv epv;
};

GType     knc_c_file_get_type (void);
KncCFile *knc_c_file_new      (KncCntrl *cntrl, unsigned long n,
			       CORBA_Environment *);

G_END_DECLS

#endif
