/* gtkam-tree-item-dir.h
 *
 * Copyright (C) 2002 Lutz M�ller <lutz@users.sourceforge.net>
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
#ifndef __GTKAM_TREE_ITEM_DIR_H__
#define __GTKAM_TREE_ITEM_DIR_H__

#include <gphoto2/gphoto2-camera.h>
#include <gtkam-tree-item.h>

#define GTKAM_TYPE_TREE_ITEM_DIR  (gtkam_tree_item_dir_get_type ())
#define GTKAM_TREE_ITEM_DIR(o)    (GTK_CHECK_CAST((o),GTKAM_TYPE_TREE_ITEM_DIR,GtkamTreeItemDir))
#define GTKAM_IS_TREE_ITEM_DIR(o) (GTK_CHECK_TYPE((o),GTKAM_TYPE_TREE_ITEM_DIR))

typedef struct _GtkamTreeItemDir        GtkamTreeItemDir;
typedef struct _GtkamTreeItemDirPrivate GtkamTreeItemDirPrivate;
typedef struct _GtkamTreeItemDirClass   GtkamTreeItemDirClass;

struct _GtkamTreeItemDir {
	GtkamTreeItem parent;

	GtkamTreeItemDirPrivate *priv;
};

struct _GtkamTreeItemDirClass {
	GtkamTreeItemClass parent_class;
};

GtkType    gtkam_tree_item_dir_get_type (void);
GtkWidget *gtkam_tree_item_dir_new      (void);

#endif /* __GTKAM_TREE_ITEM_DIR_H__ */
