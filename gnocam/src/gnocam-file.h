
#ifndef _GNOCAM_FILE_H_
#define _GNOCAM_FILE_H_

#include <gphoto2.h>
#include <bonobo.h>
#include <GnoCam.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_FILE		(gnocam_file_get_type ())
#define GNOCAM_FILE(obj)		(GTK_CHECK_CAST ((obj), GNOCAM_TYPE_FILE, GnoCamFile))
#define GNOCAM_FILE_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GNOCAM_TYPE_FILE, GnoCamFileClass))
#define GNOCAM_IS_FILE(obj)		(GTK_CHECK_TYPE ((obj), GNOCAM_TYPE_FILE))
#define GNOCAM_IS_FILE_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_FILE))

typedef struct _GnoCamFile		GnoCamFile;
typedef struct _GnoCamFilePrivate	GnoCamFilePrivate;
typedef struct _GnoCamFileClass	GnoCamFileClass;

struct _GnoCamFile {
	BonoboXObject 			parent;

	GnoCamFilePrivate*	priv;
};

struct _GnoCamFileClass {
	BonoboXObjectClass	 	parent_class;

	POA_GNOME_GnoCam_file__epv 	epv;
};


GtkType    		gnocam_file_get_type	(void);
GnoCamFile*		gnocam_file_new		(Camera* camera, Bonobo_Storage storage, const gchar* path, BonoboUIContainer* container, CORBA_Environment* ev);

void 			gnocam_file_set_ui_container 	(GnoCamFile* file, BonoboUIContainer* container);
BonoboUIComponent*	gnocam_file_get_ui_component	(GnoCamFile* file);

GtkWidget*		gnocam_file_get_widget		(GnoCamFile* file);

END_GNOME_DECLS

#endif /* _GNOCAM_FILE_H_ */

