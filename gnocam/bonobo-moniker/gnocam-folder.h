
#ifndef __GNOCAM_FOLDER_H__
#define __GNOCAM_FOLDER_H__

#include <gphoto2.h>
#include <bonobo.h>
#include <GnoCam.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_FOLDER		(gnocam_folder_get_type ())
#define GNOCAM_FOLDER(obj)		(GTK_CHECK_CAST ((obj), GNOCAM_TYPE_FOLDER, GnoCamFolder))
#define GNOCAM_FOLDER_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GNOCAM_TYPE_FOLDER, GnoCamFolderClass))
#define GNOCAM_IS_FOLDER(obj)		(GTK_CHECK_TYPE ((obj), GNOCAM_TYPE_FOLDER))
#define GNOCAM_IS_FOLDER_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_FOLDER))

typedef struct _GnoCamFolder		GnoCamFolder;
typedef struct _GnoCamFolderPrivate	GnoCamFolderPrivate;
typedef struct _GnoCamFolderClass	GnoCamFolderClass;

struct _GnoCamFolder {
	BonoboXObject		parent;

	GnoCamFolderPrivate*	priv;
};

struct _GnoCamFolderClass {
	BonoboXObjectClass              parent_class;
	POA_GNOME_GnoCam_folder__epv	epv;
};


GtkType    	gnocam_folder_get_type		(void);
GnoCamFolder*	gnocam_folder_new		(Camera* camera, Bonobo_Storage storage, const gchar* path, BonoboControl* control);

void			gnocam_folder_set_ui_container  	(GnoCamFolder* folder, Bonobo_UIContainer container);
void			gnocam_folder_unset_ui_container	(GnoCamFolder* folder);
BonoboUIComponent*	gnocam_folder_get_ui_component		(GnoCamFolder* folder);

GtkWidget*      	gnocam_folder_get_widget        	(GnoCamFolder* folder);


END_GNOME_DECLS

#endif /* __GNOCAM_FOLDER_H__ */
