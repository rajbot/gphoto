
#ifndef _GNOCAM_STORAGE_VIEW_H_
#define _GNOCAM_STORAGE_VIEW_H_

#include "gnocam-camera.h"
#include <gal/e-table/e-tree-scrolled.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_STORAGE_VIEW		(gnocam_storage_view_get_type ())
#define GNOCAM_STORAGE_VIEW(obj)		(GTK_CHECK_CAST ((obj), GNOCAM_TYPE_STORAGE_VIEW, GnoCamStorageView))
#define GNOCAM_STORAGE_VIEW_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GNOCAM_TYPE_STORAGE_VIEW, GnoCamStorageViewClass))
#define GNOCAM_IS_STORAGE_VIEW(obj)		(GTK_CHECK_TYPE ((obj), GNOCAM_TYPE_STORAGE_VIEW))
#define GNOCAM_IS_STORAGE_VIEW_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_STORAGE_VIEW))

typedef struct _GnoCamStorageView		GnoCamStorageView;
typedef struct _GnoCamStorageViewPrivate	GnoCamStorageViewPrivate;
typedef struct _GnoCamStorageViewClass		GnoCamStorageViewClass;

struct _GnoCamStorageView {
	ETreeScrolled			parent;
	
	GnoCamStorageViewPrivate*	priv;
};

struct _GnoCamStorageViewClass {
	ETreeScrolledClass		parent;

	void (* directory_selected)	(GnoCamStorageView* storage_view, const gchar* path);
	void (* file_selected)		(GnoCamStorageView* storage_view, const gchar* path);
};

GtkType		gnocam_storage_view_get_type 		(void);
GtkWidget*	gnocam_storage_view_new			(GnoCamCamera* camera);

END_GNOME_DECLS

#endif /* _GNOCAM_STORAGE_VIEW_H_ */

