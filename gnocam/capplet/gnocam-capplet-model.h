#ifndef _GNOCAM_CAPPLET_MODEL_H_
#define _GNOCAM_CAPPLET_MODEL_H_

#include <gal/e-table/e-table-model.h>
#include <capplet-widget.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_CAPPLET_MODEL		(gnocam_capplet_model_get_type ())
#define GNOCAM_CAPPLET_MODEL(obj)		(GTK_CHECK_CAST ((obj), GNOCAM_TYPE_CAPPLET_MODEL, GnoCamCappletModel))
#define GNOCAM_CAPPLET_MODEL_CLASS(klass)	(GTK_CHECK_CLASS_CAST((klass), GNOCAM_TYPE_CAPPLET_TABLE, GnoCamCappletClass))
#define GNOCAM_IS_CAPPLET_MODEL(obj)		(GTK_CHECK_TYPE ((obj), GNOCAM_TYPE_CAPPLET_MODEL))
#define GNOCAM_IS_CAPPLET_MODEL_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), GNOCAM_TYPE_CAPPLET_MODEL))

typedef struct _GnoCamCappletModel		GnoCamCappletModel;
typedef struct _GnoCamCappletModelPrivate	GnoCamCappletModelPrivate;
typedef struct _GnoCamCappletModelClass		GnoCamCappletModelClass;

struct _GnoCamCappletModel
{
	ETableModel			 parent;

	GnoCamCappletModelPrivate	*priv;
};

struct _GnoCamCappletModelClass
{
	ETableModelClass		 parent_class;
};

GtkType		 gnocam_capplet_model_get_type 	(void);
ETableModel	*gnocam_capplet_model_new 	(CappletWidget *capplet);

void		 gnocam_capplet_model_delete_row	(GnoCamCappletModel *model, gint row);

void		 gnocam_capplet_model_ok	(GnoCamCappletModel *);
void		 gnocam_capplet_model_revert	(GnoCamCappletModel *);
void		 gnocam_capplet_model_try	(GnoCamCappletModel *);
void		 gnocam_capplet_model_cancel	(GnoCamCappletModel *);

END_GNOME_DECLS

#endif /* _GNOCAM_CAPPLET_MODEL_H_ */
