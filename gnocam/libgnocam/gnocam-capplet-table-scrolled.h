#ifndef _GNOCAM_CAPPLET_TABLE_SCROLLED_H_
#define _GNOCAM_CAPPLET_TABLE_SCROLLED_H_

#include <capplet-widget.h>
#include <gal/widgets/e-scroll-frame.h>
#include <gal/e-table/e-table.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_CAPPLET_TABLE_SCROLLED         (gnocam_capplet_table_scrolled_get_type ())
#define GNOCAM_CAPPLET_TABLE_SCROLLED(obj)         (GTK_CHECK_CAST ((obj), GNOCAM_TYPE_CAPPLET_TABLE_SCROLLED, GnoCamCappletTableScrolled))
#define GNOCAM_CAPPLET_TABLE_SCROLLED_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GNOCAM_TYPE_CAPPLET_TABLE_SCROLLED, GnoCamCappletTableScrolledClass))

typedef struct _GnoCamCappletTableScrolled        GnoCamCappletTableScrolled;
typedef struct _GnoCamCappletTableScrolledPrivate GnoCamCappletTableScrolledPrivate;
typedef struct _GnoCamCappletTableScrolledClass   GnoCamCappletTableScrolledClass;

struct _GnoCamCappletTableScrolled {
	EScrollFrame parent;

	ETable *table;

	GnoCamCappletTableScrolledPrivate *priv;
};

struct _GnoCamCappletTableScrolledClass {
	EScrollFrameClass parent_class;
};

GtkType    gnocam_capplet_table_scrolled_get_type (void);
GtkWidget *gnocam_capplet_table_scrolled_new (CappletWidget *capplet);

void    gnocam_capplet_table_scrolled_ok     (GnoCamCappletTableScrolled *);
void    gnocam_capplet_table_scrolled_revert (GnoCamCappletTableScrolled *);
void    gnocam_capplet_table_scrolled_try    (GnoCamCappletTableScrolled *);
void    gnocam_capplet_table_scrolled_cancel (GnoCamCappletTableScrolled *);

END_GNOME_DECLS

#endif /* _GNOCAM_CAPPLET_TABLE_SCROLLED_H_ */
