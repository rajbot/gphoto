#ifndef _GNOCAM_CAPPLET_TABLE_H_
#define _GNOCAM_CAPPLET_TABLE_H_

#include <capplet-widget.h>
#include <gal/e-table/e-table.h>
#include <gal/e-table/e-cell-text.h>
#include <gal/e-table/e-cell-popup.h>
#include <gal/e-table/e-cell-combo.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_CAPPLET_TABLE         (gnocam_capplet_table_get_type ())
#define GNOCAM_CAPPLET_TABLE(obj)         (GTK_CHECK_CAST ((obj), GNOCAM_TYPE_CAPPLET_TABLE, GnoCamCappletTable))
#define GNOCAM_CAPPLET_TABLE_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GNOCAM_TYPE_CAPPLET_TABLE, GnoCamCappletTableClass))

typedef struct _GnoCamCappletTable        GnoCamCappletTable;
typedef struct _GnoCamCappletTablePrivate GnoCamCappletTablePrivate;
typedef struct _GnoCamCappletTableClass   GnoCamCappletTableClass;

struct _GnoCamCappletTable {
	ETable parent;

	GnoCamCappletTablePrivate *priv;
};

struct _GnoCamCappletTableClass {
	ETableClass parent_class;
};

GtkType gnocam_capplet_table_get_type (void);
ETable *gnocam_capplet_table_new (CappletWidget *capplet);

void    gnocam_capplet_table_ok     (GnoCamCappletTable *table);
void    gnocam_capplet_table_revert (GnoCamCappletTable *table);
void    gnocam_capplet_table_try    (GnoCamCappletTable *table);
void    gnocam_capplet_table_cancel (GnoCamCappletTable *table);

END_GNOME_DECLS

#endif /* _GNOCAM_CAPPLET_TABLE_H_ */
