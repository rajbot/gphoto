
#ifndef _GNOCAM_CONTROL_FOLDER_H_
#define _GNOCAM_CONTROL_FOLDER_H_

#include "gnocam-control.h"

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_CONTROL_FOLDER		(gnocam_control_folder_get_type ())
#define GNOCAM_CONTROL_FOLDER(obj)		(GTK_CHECK_CAST ((obj), GNOCAM_TYPE_CONTROL_FOLDER, GnoCamControlFolder))
#define GNOCAM_CONTROL_FOLDER_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GNOCAM_TYPE_CONTROL_FOLDER, GnoCamControlFolderClass))
#define GNOCAM_IS_CONTROL_FOLDER(obj)		(GTK_CHECK_TYPE ((obj), GNOCAM_TYPE_CONTROL_FOLDER))
#define GNOCAM_IS_CONTROL_FOLDER_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_FOLDER))

typedef struct _GnoCamControlFolder		GnoCamControlFolder;
typedef struct _GnoCamControlFolderPrivate	GnoCamControlFolderPrivate;
typedef struct _GnoCamControlFolderClass	GnoCamControlFolderClass;

struct _GnoCamControlFolder {
	GnoCamControl           	parent;

	GnoCamControlFolderPrivate*	priv;
};

struct _GnoCamControlFolderClass {
	GnoCamControlClass 	parent_class;
};


GtkType    		gnocam_control_folder_get_type		(void);
GnoCamControlFolder*	gnocam_control_folder_new		(BonoboMoniker* moniker, Bonobo_Storage storage, CORBA_Environment* ev);

END_GNOME_DECLS

#endif /* _GNOCAM_CONTROL_FOLDER_H_ */
