/* gnocam-util.h
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

#ifndef __GNOCAM_UTIL_H__
#define __GNOCAM_UTIL_H__

#include "GnoCam.h"

G_BEGIN_DECLS

#define CR(result,ev)         G_STMT_START{			              \
	gint	r;							      \
									      \
	if (!BONOBO_EX (ev) && ((r = result) < 0)) {			      \
                switch (r) {						      \
                case GP_ERROR_IO:                                             \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION,        \
					 ex_GNOME_GnoCam_IOError, NULL);      \
                        break;                                                \
                case GP_ERROR_DIRECTORY_NOT_FOUND:                            \
                case GP_ERROR_FILE_NOT_FOUND:                                 \
                case GP_ERROR_MODEL_NOT_FOUND:                                \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION,        \
					 ex_GNOME_GnoCam_NotFound, NULL);     \
                        break;                                                \
                case GP_ERROR_NOT_SUPPORTED:                                  \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION,        \
				         ex_GNOME_GnoCam_NotSupported, NULL); \
                        break;                                                \
                default:                                                      \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION,        \
					ex_GNOME_GnoCam_IOError, NULL);	      \
                        break;                                                \
                }                                                             \
        }                               }G_STMT_END

gchar *gnocam_build_path (const gchar *path, const gchar *path_to_append);

G_END_DECLS

#endif /* __GNOCAM_UTIL_H__ */
