
#ifndef _GNOCAM_CONTROL_FILE_H_
#define _GNOCAM_CONTROL_FILE_H_

#include "gnocam-control.h"

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

#define GNOCAM_TYPE_CONTROL_FILE		(gnocam_control_file_get_type ())
#define GNOCAM_CONTROL_FILE(obj)		(GTK_CHECK_CAST ((obj), GNOCAM_TYPE_CONTROL_FILE, GnoCamControlFile))
#define GNOCAM_CONTROL_FILE_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GNOCAM_TYPE_CONTROL_FILE, GnoCamControlFileClass))
#define GNOCAM_IS_CONTROL_FILE(obj)		(GTK_CHECK_TYPE ((obj), GNOCAM_TYPE_CONTROL_FILE))
#define GNOCAM_IS_CONTROL_FILE_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_FILE))

typedef struct _GnoCamControlFile		GnoCamControlFile;
typedef struct _GnoCamControlFilePrivate	GnoCamControlFilePrivate;
typedef struct _GnoCamControlFileClass	GnoCamControlFileClass;

struct _GnoCamControlFile {
	GnoCamControl 			parent;

	GnoCamControlFilePrivate*	priv;
};

struct _GnoCamControlFileClass {
	GnoCamControlClass 	parent_class;
};


GtkType    		gnocam_control_file_get_type	(void);
GnoCamControlFile*	gnocam_control_file_new		(BonoboMoniker* moniker, const Bonobo_ResolveOptions* options, Bonobo_Stream stream, CORBA_Environment* ev);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _GNOCAM_CONTROL_FILE_H_ */

