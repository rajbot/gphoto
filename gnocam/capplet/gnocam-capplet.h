
#ifndef __GNOCAM_CAPPLET_H__
#define __GNOCAM_CAPPLET_H__

#include <gconf/gconf-client.h>
#include <gtk/gtkdialog.h>

#define GNOCAM_TYPE_CAPPLET		(gnocam_capplet_get_type ())
#define GNOCAM_CAPPLET(obj)   		GTK_CHECK_CAST (obj, gnocam_capplet_get_type (), GnoCamCapplet)
#define GNOCAM_CAPPLET_CLASS(klass)		GTK_CHECK_CLASS_CAST (klass, gnocam_capplet_get_type (), GnoCamCappletClass)
#define GNOCAM_IS_CAPPLET(obj)     	GTK_CHECK_TYPE (obj, gnocam_capplet_get_type ())
#define GNOCAM_IS_CAPPLET_CLASS(klass)	GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_CAPPLET)

typedef struct _GnoCamCapplet		GnoCamCapplet;
typedef struct _GnoCamCappletPrivate	GnoCamCappletPrivate;
typedef struct _GnoCamCappletClass    	GnoCamCappletClass;

struct _GnoCamCapplet
{
	GtkDialog parent;

	GnoCamCappletPrivate*	priv;
};

struct _GnoCamCappletClass
{
	GtkDialogClass parent_class;
};

GtkType	   gnocam_capplet_get_type (void);
GtkWidget *gnocam_capplet_new	(GConfClient *client);

#endif /* __GNOCAM_CAPPLET_H__ */
