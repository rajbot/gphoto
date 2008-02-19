/** bayer.h
 *
 * Copyright 2001 Lutz Mller <lutz@users.sf.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __BAYER_H__
#define __BAYER_H__

/*
 * This enumeration defines how the CCD bayer array is laid out.
 */


typedef enum {
	BAYER_TILE_RGGB,
	BAYER_TILE_GRBG,
	BAYER_TILE_BGGR,
	BAYER_TILE_GBRG,
	BAYER_TILE_RGGB_INTERLACED,
	BAYER_TILE_GRBG_INTERLACED,
	BAYER_TILE_BGGR_INTERLACED,
	BAYER_TILE_GBRG_INTERLACED
} BayerTile;

int
gp_bayer_decode (unsigned char *input, int w, int h, unsigned char *output,
		 BayerTile tile);
#endif /* __BAYER_H__ */
