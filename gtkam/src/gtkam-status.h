/* gtkam-status.h
 *
 * Copyright 2002 Lutz Mueller <lutz@users.sf.net>
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

#ifndef __GTKAM_STATUS_H__
#define __GTKAM_STATUS_H__

#include <gtkam-context.h>
#include <gtk/gtkhbox.h>

#define GTKAM_TYPE_STATUS  (gtkam_status_get_type ())
#define GTKAM_STATUS(o)    (GTK_CHECK_CAST((o),GTKAM_TYPE_STATUS,GtkamStatus))
#define GTKAM_IS_STATUS(o) (GTK_CHECK_TYPE((o),GTKAM_TYPE_STATUS))

typedef struct _GtkamStatus        GtkamStatus;
typedef struct _GtkamStatusPrivate GtkamStatusPrivate;
typedef struct _GtkamStatusClass   GtkamStatusClass;

struct _GtkamStatus
{
	GtkHBox parent;

	GtkamContext *context;

	GtkamStatusPrivate *priv;
};

struct _GtkamStatusClass
{
	GtkHBoxClass parent_class;
};

GtkType    gtkam_status_get_type (void);
GtkWidget *gtkam_status_new      (const gchar *format, ...);

#endif /* __GTKAM_STATUS_H__ */
