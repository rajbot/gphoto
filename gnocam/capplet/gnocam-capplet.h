
#ifndef __GNOCAM_CAPPLET_H__
#define __GNOCAM_CAPPLET_H__

#include <gconf/gconf-client.h>
#include <gtk/gtkdialog.h>

#define GNOCAM_TYPE_CAPPLET		(gnocam_capplet_get_type ())
#define GNOCAM_CAPPLET(obj)   		GTK_CHECK_CAST (obj, gnocam_capplet_get_type (), GnocamCapplet)
#define GNOCAM_CAPPLET_CLASS(klass)		GTK_CHECK_CLASS_CAST (klass, gnocam_capplet_get_type (), GnocamCappletClass)
#define GNOCAM_IS_CAPPLET(obj)     	GTK_CHECK_TYPE (obj, gnocam_capplet_get_type ())
#define GNOCAM_IS_CAPPLET_CLASS(klass)	GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_CAPPLET)

typedef struct _GnocamCapplet		GnocamCapplet;
typedef struct _GnocamCappletPrivate	GnocamCappletPrivate;
typedef struct _GnocamCappletClass    	GnocamCappletClass;

struct _GnocamCapplet
{
	GtkDialog parent;

	GnocamCappletPrivate*	priv;
};

struct _GnocamCappletClass
{
	GtkDialogClass parent_class;
};

GtkType	   gnocam_capplet_get_type (void);
GtkWidget *gnocam_capplet_new	(GConfClient *client);

#endif /* __GNOCAM_CAPPLET_H__ */
