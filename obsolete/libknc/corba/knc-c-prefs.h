#ifndef __KNC_C_PREFS_H__
#define __KNC_C_PREFS_H__

#include <bonobo/bonobo-control.h>
#include <libknc/knc-cntrl.h>

G_BEGIN_DECLS

#define KNC_C_TYPE_PREFS (knc_c_prefs_get_type ())
#define KNC_C_PREFS(o) (G_TYPE_CHECK_INSTANCE_CAST((o),KNC_C_TYPE_PREFS,KncCPrefs))
#define KNC_C_PREFS_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k),KNC_C_TYPE_PREFS,KncCPrefsClass))
#define KNC_C_IS_PREFS(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),KNC_C_TYPE_PREFS))

typedef struct _KncCPrefs      KncCPrefs;
typedef struct _KncCPrefsClass KncCPrefsClass;
typedef struct _KncCPrefsPriv  KncCPrefsPriv;

struct _KncCPrefs {
	BonoboControl parent;

	KncCPrefsPriv *priv;
};

struct _KncCPrefsClass {
	BonoboControlClass parent_class;
};

GType      knc_c_prefs_get_type (void);
KncCPrefs *knc_c_prefs_new      (KncCntrl *);

G_END_DECLS

#endif
