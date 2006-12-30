/* gnocam-util.c
 *
 * Copyright � 2002 Lutz M�ller <lutz@users.sourceforge.net>
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
#include "gnocam-util.h"

#include <string.h>

gchar *
gnocam_build_path (const gchar *path, const gchar *path_to_append)
{
	if (!path) {
		if (!path_to_append)
			return NULL;
		else
			return (g_strdup (path_to_append));
	}


	if (!path_to_append)
		return (g_strdup (path));

	if (*path_to_append == '/')
		return (g_strdup (path_to_append));

        if (path[strlen (path) - 1] == '/')
		return (g_strdup_printf ("%s%s", path, path_to_append));

	return (g_strdup_printf ("%s/%s", path, path_to_append));
}
