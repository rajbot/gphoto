/*
 *	File: pixmaps.h
 *
 *	Copyright (C) 1998 Ugo Paternostro <paterno@dsi.unifi.it>
 *
 *	This file is part of the dc20ctrl package. The complete package can be
 *	downloaded from:
 *	    http://aguirre.dsi.unifi.it/~paterno/binaries/dc20ctrl.tar.gz
 *
 *	This package is derived from the dc20 package, built by Karl Hakimian
 *	<hakimian@aha.com> that you can find it at ftp.eecs.wsu.edu in the
 *	/pub/hakimian directory. The complete URL is:
 *	    ftp://ftp.eecs.wsu.edu/pub/hakimian/dc20.tar.gz
 *
 *	This package also includes a sligthly modified version of the Comet to ppm
 *	conversion routine written by YOSHIDA Hideki <hideki@yk.rim.or.jp>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published 
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *	Prototypes for pixmaps.c
 *
 *	$Id$
 */

#ifndef _PIXMAPS_H_
#define _PIXMAPS_H_

#include "dc20.h"

struct pixmap *alloc_pixmap(int, int, int);
void free_pixmap(struct pixmap *);
int save_pixmap_pxm(struct pixmap *, FILE *);
int set_pixel(struct pixmap *, int, int, unsigned char);
int set_pixel_rgb(struct pixmap *, int, int, unsigned char, unsigned char, unsigned char);
int zoom_x(struct pixmap *, struct pixmap *);
int zoom_y(struct pixmap *, struct pixmap *);
#ifdef USE_JPEG
int save_pixmap_jpeg(struct pixmap *, int, FILE *);
#endif /* USE_JPEG */
struct pixmap *rotate_right(struct pixmap *);
struct pixmap *rotate_left(struct pixmap *);
#ifdef USE_TIFF
int save_pixmap_tiff(struct pixmap *, int, int, int, char *);
#endif /* USE_TIFF */
#ifdef USE_PNG
int save_pixmap_png(struct pixmap *, FILE *);
#endif /* USE_PNG */
int save_pixmap(struct pixmap *, char *, int, int);

#endif /* _PIXMAPS_H_ */
