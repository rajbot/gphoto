/* gphoto2-ftp-type.c
 *
 * Copyright © 2002 Lutz Müller <lutz@users.sourceforge.net>
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

#include "config.h"
#include "gphoto2-ftp-type.h"

#include <stdio.h>
#include <ctype.h>

#include <gphoto2-ftp-params.h>

static struct {
	GFType type;
	const char *name;
} GFTypes[] = {
	{GF_TYPE_ASCII, "ASCII"},
	{GF_TYPE_BINARY, "BINARY"},
	{GF_TYPE_LOCAL, "LOCAL"},
	{0, NULL}
};

void
gf_type (GFParams *params, char type)
{
	unsigned int i;

	switch (tolower (type)) {
	case 'i':
		params->type = GF_TYPE_BINARY;
		break;
	case 'a':
		params->type = GF_TYPE_ASCII;
		break;
	case 'l':
		params->type = GF_TYPE_LOCAL;
		break;
	default:
		fprintf (stdout, "504 Unknown type '%c'.\r\n", type);
		fflush (stdout);
		break;
	}

	for (i = 0; GFTypes[i].name; i++)
		if (GFTypes[i].type == params->type)
			break;
	fprintf (stdout, "  0 Type is now '%s'.\r\n", GFTypes[i].name);
	fflush (stdout);
}
