#ifndef __GNOCAM_CHOOSER_H__
#define __GNOCAM_CHOOSER_H__

#include <gtk/gtkdialog.h>

#define GNOCAM_TYPE_CHOOSER (gnocam_chooser_get_type())
#define GNOCAM_CHOOSER(o) (GTK_CHECK_CAST((o),GNOCAM_TYPE_CHOOSER,GnocamChooser))
#define GNOCAM_IS_CHOOSER(o) (GTK_CHECK_TYPE((o),GNOCAM_TYPE_CHOOSER))

typedef struct _GnocamChooser      GnocamChooser;
typedef struct _GnocamChooserPriv  GnocamChooserPriv;
typedef struct _GnocamChooserClass GnocamChooserClass;

struct _GnocamChooser
{
	GtkDialog parent;

	GnocamChooserPriv *priv;
};

struct _GnocamChooserClass
{
	GtkDialogClass parent_class;
};

GtkType        gnocam_chooser_get_type (void);
GnocamChooser *gnocam_chooser_new      (void);

#endif /* __GNOCAM_CHOOSER_H__ */
