/* gtkam-port.h
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

#ifndef __GTKAM_PORT_H__
#define __GTKAM_PORT_H__

#include <gtkam-dialog.h>

#define GTKAM_TYPE_PORT  (gtkam_port_get_type ())
#define GTKAM_PORT(o)    (GTK_CHECK_CAST((o),GTKAM_TYPE_PORT,GtkamPort))
#define GTKAM_IS_PORT(o) (GTK_CHECK_TYPE((o),GTKAM_TYPE_PORT))

typedef struct _GtkamPort        GtkamPort;
typedef struct _GtkamPortPrivate GtkamPortPrivate;
typedef struct _GtkamPortClass   GtkamPortClass;

struct _GtkamPort
{
	GtkamDialog parent;

	GtkamPortPrivate *priv;
};

struct _GtkamPortClass
{
	GtkamDialogClass parent_class;

	/* Signals */
	void (* port_added) (GtkamPort *port, const gchar *path);
};

GtkType    gtkam_port_get_type (void);
GtkWidget *gtkam_port_new      (GtkWidget *opt_window);

#endif /* __GTKAM_PORT_H__ */
