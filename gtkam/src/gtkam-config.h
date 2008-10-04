/* gtkam-config.h
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

#ifndef __GTKAM_CONFIG_H__
#define __GTKAM_CONFIG_H__

#include <gtkam-camera.h>
#include <gtkam-dialog.h>

#define GTKAM_TYPE_CONFIG  (gtkam_config_get_type ())
#define GTKAM_CONFIG(o)    (GTK_CHECK_CAST((o),GTKAM_TYPE_CONFIG,GtkamConfig))
#define GTKAM_IS_CONFIG(o) (GTK_CHECK_TYPE((o),GTKAM_TYPE_CONFIG))

typedef struct _GtkamConfig        GtkamConfig;
typedef struct _GtkamConfigPrivate GtkamConfigPrivate;
typedef struct _GtkamConfigClass   GtkamConfigClass;

struct _GtkamConfig
{
	GtkamDialog parent;

	GtkamConfigPrivate *priv;
};

struct _GtkamConfigClass
{
	GtkamDialogClass parent_class;
};

GtkType    gtkam_config_get_type (void);
GtkWidget *gtkam_config_new      (GtkamCamera *camera);

#endif /* __GTKAM_CONFIG_H__ */
