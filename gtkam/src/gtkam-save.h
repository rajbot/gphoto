/* gtkam-save.h
 *
 * Copyright (C) 2001 Lutz M�ller <urc8@rz.uni-karlsruhe.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GTKAM_SAVE_H__
#define __GTKAM_SAVE_H__

#include <gphoto2/gphoto2-camera.h>
#include <gtk/gtkfilesel.h>

#define GTKAM_TYPE_SAVE  (gtkam_save_get_type ())
#define GTKAM_SAVE(o)    (GTK_CHECK_CAST((o),GTKAM_TYPE_SAVE,GtkamSave))
#define GTKAM_IS_SAVE(o) (GTK_CHECK_TYPE((o),GTKAM_TYPE_SAVE))

typedef struct _GtkamSave        GtkamSave;
typedef struct _GtkamSavePrivate GtkamSavePrivate;
typedef struct _GtkamSaveClass   GtkamSaveClass;

struct _GtkamSave
{
	GtkFileSelection parent;

	GtkamSavePrivate *priv;
};

struct _GtkamSaveClass
{
	GtkFileSelectionClass parent_class;
};

GtkType    gtkam_save_get_type (void);
GtkWidget *gtkam_save_new      (Camera *camera, gboolean multi,
				const gchar *path, GSList *filenames,
				GtkWidget *opt_window);

#endif /* __GTKAM_SAVE_H__ */
