/* gnocam-main.h
 *
 * Copyright (C) 2002 Lutz Müller <lutz@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
#ifndef _GNOCAM_MAIN_H_
#define _GNOCAM_MAIN_H_

#include <glib/gmacros.h>

#include <GnoCam.h>

#include <bonobo/bonobo-object.h>

#include <gnocam-cache.h>

G_BEGIN_DECLS

#define GNOCAM_TYPE_MAIN	(gnocam_main_get_type())
#define GNOCAM_MAIN(o)          (G_TYPE_CHECK_INSTANCE_CAST((o),GNOCAM_TYPE_MAIN,GnoCamMain))
#define GNOCAM_MAIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k),GNOCAM_TYPE_MAIN,GnoCamMainClass))
#define GNOCAM_IS_MAIN(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o),GNOCAM_TYPE_MAIN))
#define GNOCAM_IS_MAIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k),GNOCAM_TYPE_MAIN))

typedef struct _GnoCamMain		GnoCamMain;
typedef struct _GnoCamMainPrivate	GnoCamMainPrivate;
typedef struct _GnoCamMainClass    	GnoCamMainClass;

struct _GnoCamMain {
	BonoboObject parent;

	GnoCamMainPrivate *priv;
};

struct _GnoCamMainClass {
	BonoboObjectClass parent_class;

	POA_GNOME_GnoCam__epv epv;
};

GType       gnocam_main_get_type (void);
GnoCamMain *gnocam_main_new	 (GnoCamCache *);

G_END_DECLS

#endif /* _GNOCAM__MAIN_H_ */
