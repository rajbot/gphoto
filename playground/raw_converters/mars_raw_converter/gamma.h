/* gamma.h
 *
 * Copyright � 2001 Lutz M�ller <lutz@users.sf.net>
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

#ifndef __GAMMA_H__
#define __GAMMA_H__

int gp_gamma_fill_table     (unsigned char *table, double g);
int gp_gamma_correct_single (unsigned char *table, unsigned char *data, 
			     unsigned int data_size);
int gp_gamma_correct_triple (unsigned char *table_red,
			 unsigned char *table_green,
			 unsigned char *table_blue,
			 unsigned char *data, unsigned int size);

#endif /* __GAMMA_H__ */
