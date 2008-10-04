/* gtkam-delete.h
 *
 * Copyright 2001,2002 Lutz Mueller <lutz@users.sf.net>
 * Copyright 2004,2005 Andrew Burton <adburton@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GTKAM_DELETE_H__
#define __GTKAM_DELETE_H__

#include <gtkam-camera.h>
#include <gtk/gtkdialog.h>

#define GTKAM_TYPE_DELETE  (gtkam_delete_get_type ())
#define GTKAM_DELETE(o)    (GTK_CHECK_CAST((o),GTKAM_TYPE_DELETE,GtkamDelete))
#define GTKAM_IS_DELETE(o) (GTK_CHECK_TYPE((o),GTKAM_TYPE_DELETE))

typedef struct _GtkamDelete        GtkamDelete;
typedef struct _GtkamDeletePrivate GtkamDeletePrivate;
typedef struct _GtkamDeleteClass   GtkamDeleteClass;

struct _GtkamDelete
{
	GtkDialog parent;

	GtkamDeletePrivate *priv;
};

typedef struct _GtkamDeleteFileDeletedData GtkamDeleteFileDeletedData;
struct _GtkamDeleteFileDeletedData {
	GtkamCamera *camera;
	const gchar *folder;
	const gchar *name;
};

typedef struct _GtkamDeleteAllDeletedData GtkamDeleteAllDeletedData;
struct _GtkamDeleteAllDeletedData {
	GtkamCamera *camera;
	const gchar *folder;
};

struct _GtkamDeleteClass
{
	GtkDialogClass parent_class;

	/* Signals */
	void (* all_deleted)  (GtkamDelete *, GtkamDeleteAllDeletedData *);
	void (* file_deleted) (GtkamDelete *, GtkamDeleteFileDeletedData *);
};

GtkType    gtkam_delete_get_type (void);
GtkWidget *gtkam_delete_new      (void);

void       gtkam_delete_add      (GtkamDelete *delete, GtkamCamera *camera,
				  const gchar *folder, const gchar *name, gboolean all);

#endif /* __GTKAM_DELETE_H__ */
