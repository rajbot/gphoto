
#ifndef _GNOCAM_MAIN_H_
#define _GNOCAM_MAIN_H_

#include <gphoto2.h>
#include <bonobo.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_MAIN		(gnocam_main_get_type ())
#define GNOCAM_MAIN(obj)		(GTK_CHECK_CAST ((obj), GNOCAM_TYPE_MAIN, GnoCamMain))
#define GNOCAM_MAIN_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GNOCAM_TYPE_MAIN, GnoCamMainClass))
#define GNOCAM_IS_MAIN(obj)		(GTK_CHECK_TYPE ((obj), GNOCAM_TYPE_MAIN))
#define GNOCAM_IS_MAIN_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_MAIN))

typedef struct _GnoCamMain		GnoCamMain;
typedef struct _GnoCamMainPrivate	GnoCamMainPrivate;
typedef struct _GnoCamMainClass		GnoCamMainClass;

struct _GnoCamMain {
	BonoboWindow		parent;
	GnoCamMainPrivate*	priv;
};

struct _GnoCamMainClass {
	BonoboWindowClass	parent_class;
};

GtkType		gnocam_main_get_type 	(void);
GnoCamMain*	gnocam_main_new 	(void);

END_GNOME_DECLS

#endif /* _GNOCAM_MAIN_H_ */

