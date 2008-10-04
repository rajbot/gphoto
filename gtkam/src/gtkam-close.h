/* gtkam-close.h
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

#ifndef __GTKAM_CLOSE_H__
#define __GTKAM_CLOSE_H__

#include <gtkam-dialog.h>

#define GTKAM_TYPE_CLOSE  (gtkam_close_get_type ())
#define GTKAM_CLOSE(o)    (GTK_CHECK_CAST((o),GTKAM_TYPE_CLOSE,GtkamClose))
#define GTKAM_IS_CLOSE(o) (GTK_CHECK_TYPE((o),GTKAM_TYPE_CLOSE))

typedef struct _GtkamClose        GtkamClose;
typedef struct _GtkamClosePrivate GtkamClosePrivate;
typedef struct _GtkamCloseClass   GtkamCloseClass;

struct _GtkamClose
{
	GtkamDialog parent;

	GtkamClosePrivate *priv;
};

struct _GtkamCloseClass
{
	GtkamDialogClass parent_class;
};

GtkType    gtkam_close_get_type (void);
GtkWidget *gtkam_close_new      (const gchar *msg);

#endif /* __GTKAM_CLOSE_H__ */
