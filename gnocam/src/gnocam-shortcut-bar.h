
#ifndef GNOCAM_SHORTCUT_BAR_H_
#define GNOCAM_SHORTCUT_BAR_H_

#include <gal/shortcut-bar/e-shortcut-bar.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _GnoCamShortcutBar		GnoCamShortcutBar;
typedef struct _GnoCamShortcutBarPrivate 	GnoCamShortcutBarPrivate;
typedef struct _GnoCamShortcutBarClass  	GnoCamShortcutBarClass;

#define GNOCAM_TYPE_SHORTCUT_BAR          (gnocam_shortcut_bar_get_type ())
#define GNOCAM_SHORTCUT_BAR(obj)          GTK_CHECK_CAST (obj, gnocam_shortcut_bar_get_type (), GnoCamShortcutBar)
#define GNOCAM_SHORTCUT_BAR_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, gnocam_shortcut_bar_get_type (), GnoCamShortcutBarClass)
#define GNOCAM_IS_SHORTCUT_BAR(obj)       GTK_CHECK_TYPE (obj, gnocam_shortcut_bar_get_type ())

struct _GnoCamShortcutBar
{
	EShortcutBar			parent;
	
	GnoCamShortcutBarPrivate*	priv;
};

struct _GnoCamShortcutBarClass
{
	EShortcutBarClass		parent_class;
};

GtkType    gnocam_shortcut_bar_get_type (void);
GtkWidget *gnocam_shortcut_bar_new      (void);

void       gnocam_shortcut_bar_refresh	(GnoCamShortcutBar* bar);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _GNOCAM_SHORTCUT_BAR_H_ */




