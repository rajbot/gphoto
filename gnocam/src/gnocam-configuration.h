
#ifndef _GNOCAM_CONFIGURATION_H_
#define _GNOCAM_CONFIGURATION_H_

#include <gphoto2.h>
#include <gnome.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_CONFIGURATION		(gnocam_configuration_get_type ())
#define GNOCAM_CONFIGURATION(obj)		GTK_CHECK_CAST (obj, gnocam_configuration_get_type (), GnoCamConfiguration)
#define GNOCAM_CONFIGURATION_CLASS(klass)	GTK_CHECK_CLASS_CAST (klass, gnocam_configuration_get_type (), GnoCamConfigurationClass)
#define GNOCAM_IS_CONFIGURATION(obj)		GTK_CHECK_TYPE (obj, gnocam_configuration_get_type ())
#define GNOCAM_IS_CONFIGURATION_CLASS(klass)	GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_CONFIGURATION)

typedef struct _GnoCamConfiguration		GnoCamConfiguration;
typedef struct _GnoCamConfigurationPrivate      GnoCamConfigurationPrivate;
typedef struct _GnoCamConfigurationClass        GnoCamConfigurationClass;

struct _GnoCamConfiguration
{
	GnomeDialog			dialog;
	
	GnoCamConfigurationPrivate*	priv;
};

struct _GnoCamConfigurationClass
{
	GnomeDialogClass		parent_class;
};

GtkType    gnocam_configuration_get_type 	(void);
GtkWidget *gnocam_configuration_new      	(Camera* camera, const gchar* dirname, const gchar* filename, GtkWidget* window);

END_GNOME_DECLS

#endif /* _GNOCAM_CONFIGURATION_H_ */




