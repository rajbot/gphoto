/* casio-qv-commands.h
 *
 * Copyright (C) 2001 Lutz M�ller
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

#ifndef __CASIO_QV_COMMANDS_H__
#define __CASIO_QV_COMMANDS_H__

#include <gphoto2-library.h>

int QVping     (Camera *camera);

/* Battery level in volts */
int QVbattery  (Camera *camera, float *battery);

int QVrevision (Camera *camera, long int *revision);
int QVnumpic   (Camera *camera);

int QVdelete   (Camera *camera, int n);
int QVpicattr  (Camera *camera, int n, unsigned char *attr);
int QVshowpic  (Camera *camera, int n);
int QVsetpic   (Camera *camera);
int QVgetpic   (Camera *camera, unsigned char *data, long int size);
int QVsize     (Camera *camera, long int *size);

int QVcapture  (Camera *camera);

#endif /* __CASIO_QV_COMMANDS_H__ */
