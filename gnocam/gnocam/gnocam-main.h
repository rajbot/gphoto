
#ifndef _GNOCAM_MAIN_H_
#define _GNOCAM_MAIN_H_

#include "GnoCam.h"
#include <bonobo/bonobo-xobject.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_MAIN		(gnocam_main_get_type ())
#define GNOCAM_MAIN(o)             	(GTK_CHECK_CAST ((o), GNOCAM_TYPE_MAIN, GnoCamMain))
#define GNOCAM_MAIN_CLASS(k)		(GTK_CHECK_CLASS_CAST((k), GNOCAM_TYPE_MAIN, GnoCamMainClass))
#define GNOCAM_IS_MAIN(o)		(GTK_CHECK_TYPE ((o), GNOCAM_TYPE_MAIN))
#define GNOCAM_IS_MAIN_CLASS(k)	(GTK_CHECK_CLASS_TYPE ((k), GNOCAM_TYPE_MAIN))

typedef struct _GnoCamMain		GnoCamMain;
typedef struct _GnoCamMainPrivate	GnoCamMainPrivate;
typedef struct _GnoCamMainClass    	GnoCamMainClass;

struct _GnoCamMain {
	BonoboXObject base;

	GnoCamMainPrivate *priv;
};

struct _GnoCamMainClass {
	BonoboXObjectClass parent_class;

	POA_GNOME_GnoCam__epv epv;
};

GtkType     gnocam_main_get_type (void);
GnoCamMain* gnocam_main_new	 (void);

END_GNOME_DECLS

#endif /* _GNOCAM__MAIN_H_ */
