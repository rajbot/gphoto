/* gphoto2-port-debug.h
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

#ifndef __GPHOTO2_PORT_DEBUG_H__
#define __GPHOTO2_PORT_DEBUG_H__

/* Debug levels */
#define GP_DEBUG_NONE           0
#define GP_DEBUG_LOW            1
#define GP_DEBUG_MEDIUM         2
#define GP_DEBUG_HIGH           3

void gp_port_debug_set_level (int level);
int  gp_port_debug_get_level (void);

void gp_port_debug_print_data (int debug_level, const char *bytes, int size);
void gp_port_debug_printf     (int debug_level, char *format, ...);

/* Custom debugging function */
typedef void (* GPPortDebugFunc) (const char *msg, void *data);
void gp_port_debug_set_func (GPPortDebugFunc func, void *data);

/* History */
void        gp_port_debug_history_append (const char *msg);
int         gp_port_debug_history_set_size (unsigned int size);
int         gp_port_debug_history_get_size (void);
const char *gp_port_debug_history_get (void);

#endif /* __GPHOTO2_PORT_DEBUG_H__ */
