
#ifndef __GNOCAM_CONTROL_FOLDER_H__
#define __GNOCAM_CONTROL_FOLDER_H__

#include <gphoto2.h>
#include <bonobo.h>

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
	BonoboControl			parent;

	GnoCamControlFolderPrivate*	priv;
};

struct _GnoCamControlFolderClass {
	BonoboControlClass		parent_class;
};


GtkType    		gnocam_control_folder_get_type		(void);
GnoCamControlFolder*	gnocam_control_folder_new		(Camera* camera, Bonobo_Storage storage, const gchar* path);

END_GNOME_DECLS

#endif /* __GNOCAM_CONTROL_FOLDER_H__ */
