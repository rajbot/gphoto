
#ifndef __GNOCAM_CAPPLET_CONTENT_H__
#define __GNOCAM_CAPPLET_CONTENT_H__

#include <gnome.h>
#include <capplet-widget.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_CAPPLET_CONTENT		(gnocam_capplet_content_get_type ())
#define GNOCAM_CAPPLET_CONTENT(obj)   		GTK_CHECK_CAST (obj, gnocam_capplet_content_get_type (), GnoCamCappletContent)
#define GNOCAM_CAPPLET_CLASS(klass)		GTK_CHECK_CLASS_CAST (klass, gnocam_capplet_content_get_type (), GnoCamCappletContentClass)
#define GNOCAM_IS_CAPPLET_CONTENT(obj)     	GTK_CHECK_TYPE (obj, gnocam_capplet_content_get_type ())
#define GNOCAM_IS_CAPPLET_CONTENT_CLASS(klass)	GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_CAPPLET_CONTENT)

typedef struct _GnoCamCappletContent		GnoCamCappletContent;
typedef struct _GnoCamCappletContentPrivate	GnoCamCappletContentPrivate;
typedef struct _GnoCamCappletContentClass    	GnoCamCappletContentClass;

struct _GnoCamCappletContent
{
	GtkVBox				parent;

	GnoCamCappletContentPrivate*	priv;
};

struct _GnoCamCappletContentClass
{
	GtkVBoxClass			parent_class;
};

GtkType		gnocam_capplet_content_get_type 	(void);
GtkWidget*	gnocam_capplet_content_new		(CappletWidget* capplet);

void		gnocam_capplet_content_ok	(GnoCamCappletContent* content);
void		gnocam_capplet_content_revert	(GnoCamCappletContent* content);
void		gnocam_capplet_content_cancel	(GnoCamCappletContent* content);
void		gnocam_capplet_content_try	(GnoCamCappletContent* content);

END_GNOME_DECLS

#endif /* __GNOCAM_CAPPLET_CONTENT_H__ */
