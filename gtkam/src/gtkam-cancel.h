/* gtkam-cancel.h
 *
 * Copyright 2001,2002 Lutz Mueller <lutz@users.sf.net>
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

#ifndef __GTKAM_CANCEL_H__
#define __GTKAM_CANCEL_H__

#include <gtkam-context.h>
#include <gtkam-dialog.h>

#define GTKAM_TYPE_CANCEL  (gtkam_cancel_get_type ())
#define GTKAM_CANCEL(o)    (GTK_CHECK_CAST((o),GTKAM_TYPE_CANCEL,GtkamCancel))
#define GTKAM_IS_CANCEL(o) (GTK_CHECK_TYPE((o),GTKAM_TYPE_CANCEL))

typedef struct _GtkamCancel        GtkamCancel;
typedef struct _GtkamCancelPrivate GtkamCancelPrivate;
typedef struct _GtkamCancelClass   GtkamCancelClass;

struct _GtkamCancel
{
	GtkamDialog parent;

	GtkamContext *context;

	GtkamCancelPrivate *priv;
};

struct _GtkamCancelClass
{
	GtkamDialogClass parent_class;

	void (* cancel) (GtkamCancel *);
};

GtkType      gtkam_cancel_get_type (void);
GtkWidget   *gtkam_cancel_new      (const gchar *format, ...);

#endif /* __GTKAM_CANCEL_H__ */
