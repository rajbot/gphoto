
#ifndef _GNOCAM_FILE_H_
#define _GNOCAM_FILE_H_

#include <gphoto2.h>
#include <bonobo.h>
#include <gconf/gconf-client.h>
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

	void (* widget_changed)		(GnoCamFile* file);
};


GtkType    	gnocam_file_get_type		(void);
GnoCamFile*	gnocam_file_new			(Camera* camera, Bonobo_Storage storage, const gchar* path, Bonobo_UIContainer container, GConfClient* client, 
						 GtkWidget* window);

void 		gnocam_file_show_menu 		(GnoCamFile* file);
void		gnocam_file_hide_menu		(GnoCamFile* file);

GtkWidget*	gnocam_file_get_widget		(GnoCamFile* file);

END_GNOME_DECLS

#endif /* _GNOCAM_FILE_H_ */

