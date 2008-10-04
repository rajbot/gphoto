/* gtkam-info.h
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

#ifndef __GTKAM_INFO_H__
#define __GTKAM_INFO_H__

#include <gtkam-camera.h>
#include <gtkam-dialog.h>

#define GTKAM_TYPE_INFO  (gtkam_info_get_type ())
#define GTKAM_INFO(o)    (GTK_CHECK_CAST((o),GTKAM_TYPE_INFO,GtkamInfo))
#define GTKAM_IS_INFO(o) (GTK_CHECK_TYPE((o),GTKAM_TYPE_INFO))

typedef struct _GtkamInfo        GtkamInfo;
typedef struct _GtkamInfoPrivate GtkamInfoPrivate;
typedef struct _GtkamInfoClass   GtkamInfoClass;

struct _GtkamInfo
{
	GtkamDialog parent;

	GtkamInfoPrivate *priv;
};

typedef struct _GtkamInfoInfoUpdatedData GtkamInfoInfoUpdatedData;
struct _GtkamInfoInfoUpdatedData {
	GtkamCamera *camera;
	const gchar *folder;
	CameraFileInfo info;
};

struct _GtkamInfoClass
{
	GtkamDialogClass parent_class;

	/* Signals */
	void (* info_updated) (GtkamInfo *info, GtkamInfoInfoUpdatedData *);
};

GtkType    gtkam_info_get_type (void);
GtkWidget *gtkam_info_new      (GtkamCamera *camera, const gchar *folder,
				const gchar *name);

#endif /* __GTKAM_INFO_H__ */
