
#ifndef GNOCAM_PREFERENCES_H_
#define GNOCAM_PREFERENCES_H_

#include <gnome.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_PREFERENCES			(gnocam_preferences_get_type ())
#define GNOCAM_PREFERENCES(obj)			GTK_CHECK_CAST (obj, gnocam_preferences_get_type (), GnoCamPreferences)
#define GNOCAM_PREFERENCES_CLASS(klass)		GTK_CHECK_CLASS_CAST (klass, gnocam_preferences_get_type (), GnoCamPreferencesClass)
#define GNOCAM_IS_PREFERENCES(obj)		GTK_CHECK_TYPE (obj, gnocam_preferences_get_type ())
#define GNOCAM_IS_PREFERENCES_CLASS(klass)	GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_PREFERENCES)

typedef struct _GnoCamPreferences               GnoCamPreferences;
typedef struct _GnoCamPreferencesPrivate        GnoCamPreferencesPrivate;
typedef struct _GnoCamPreferencesClass          GnoCamPreferencesClass;

struct _GnoCamPreferences
{
	GnomeDialog			dialog;
	
	GnoCamPreferencesPrivate*	priv;
};

struct _GnoCamPreferencesClass
{
	GnomeDialogClass		parent_class;
};

GtkType    gnocam_preferences_get_type (void);
GtkWidget *gnocam_preferences_new      (GtkWindow* parent);

END_GNOME_DECLS

#endif /* _GNOCAM_PREFERENCES_H_ */




