
#ifndef _GNOCAM_CONTROL_FOLDER_H_
#define _GNOCAM_CONTROL_FOLDER_H_

#include <bonobo.h>

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

#define GNOCAM_TYPE_CONTROL_FOLDER		(gnocam_control_folder_get_type ())
#define GNOCAM_CONTROL_FOLDER(obj)		(GTK_CHECK_CAST ((obj), GNOCAM_TYPE_CONTROL_FOLDER, GnoCamControlFolder))
#define GNOCAM_CONTROL_FOLDER_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GNOCAM_TYPE_CONTROL_FOLDER, GnoCamControlFolderClass))
#define GNOCAM_IS_CONTROL_FOLDER(obj)		(GTK_CHECK_TYPE ((obj), GNOCAM_TYPE_CONTROL_FOLDER))
#define GNOCAM_IS_CONTROL_FOLDER_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_FOLDER))

typedef struct _GnoCamControlFolder		GnoCamControlFolder;
typedef struct _GnoCamControlFolderPrivate	GnoCamControlFolderPrivate;
typedef struct _GnoCamControlFolderClass	GnoCamControlFolderClass;

struct _GnoCamControlFolder {
	BonoboControl           	parent;

	GnoCamControlFolderPrivate*	priv;
};

struct _GnoCamControlFolderClass {
	BonoboControl 	parent_class;
};


GtkType    		gnocam_control_folder_get_type		(void);
GnoCamControlFolder*	gnocam_control_folder_new		(Bonobo_Storage storage, CORBA_Environment* ev);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _GNOCAM_CONTROL_FOLDER_H_ */
