
#ifndef _GNOCAM_FILE_H_
#define _GNOCAM_FILE_H_

#include <gphoto2.h>
#include <bonobo.h>
#include <gconf/gconf-client.h>

#include "gnocam-camera.h"

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_FILE		(gnocam_file_get_type ())
#define GNOCAM_FILE(obj)		(GTK_CHECK_CAST ((obj), GNOCAM_TYPE_FILE, GnoCamFile))
#define GNOCAM_FILE_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GNOCAM_TYPE_FILE, GnoCamFileClass))
#define GNOCAM_IS_FILE(obj)		(GTK_CHECK_TYPE ((obj), GNOCAM_TYPE_FILE))
#define GNOCAM_IS_FILE_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_FILE))

typedef struct _GnoCamFile		GnoCamFile;
typedef struct _GnoCamFilePrivate	GnoCamFilePrivate;
typedef struct _GnoCamFileClass		GnoCamFileClass;

struct _GnoCamFile {
	GtkVBox			parent;

	GnoCamFilePrivate*	priv;
};

struct _GnoCamFileClass {
	GtkVBoxClass	 	parent_class;
};


GtkType    	gnocam_file_get_type		(void);
GtkWidget*	gnocam_file_new			(GnoCamCamera* camera, const gchar* path);

void 		gnocam_file_show_menu 		(GnoCamFile* file);
void		gnocam_file_hide_menu		(GnoCamFile* file);

END_GNOME_DECLS

#endif /* _GNOCAM_FILE_H_ */

