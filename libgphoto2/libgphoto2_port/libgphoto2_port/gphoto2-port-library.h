/* gphoto2-port-library.h
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

#ifndef __GPHOTO2_PORT_LIBRARY_H__
#define __GPHOTO2_PORT_LIBRARY_H__

#include <gphoto2-port-core.h>

typedef GPPortType (* GPPortLibraryType) (void);
typedef int (* GPPortLibraryList)       (GPPortInfo *, int *);

typedef GPPortOperations *(* GPPortLibraryOperations) (void);

/*
 * If you want to write an io library, you need to implement the following
 * functions. Everything else in your io library should be declared static.
 */

GPPortType gp_port_library_type       (void);
int gp_port_library_list       (GPPortInfo *info, int *count);

GPPortOperations *gp_port_library_operations (void);

#endif /* __GPHOTO2_PORT_LIBRARY_H__ */
