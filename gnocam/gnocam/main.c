/* main.c
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

#include <string.h>

#include <bonobo/bonobo-context.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-generic-factory.h>

#include <libgnomeui/gnome-ui-init.h>

#include "gnocam-main.h"

static GnoCamCache *cache = NULL;

static BonoboObject *
gnocam_factory (BonoboGenericFactory *this, const char *oaf_iid,
		gpointer data)
{
	GnoCamMain *m;

g_message ("Trying to create a new GnoCamMain...");

	if (!strcmp (oaf_iid, "OAFIID:GNOME_GnoCam")) {
		m = gnocam_main_new (cache);
		if (!m)
			return NULL;
g_message ("Done. Returning GnoCamMain.");
		return (BONOBO_OBJECT (m));
	} else {
		g_message ("Unknown OAFIID '%s'.", oaf_iid);
		return (NULL);
	}
}

int
main (int argc, char **argv)
{
	int result;

	bindtextdomain (PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	textdomain (PACKAGE);

	gnome_program_init ("gnocam", VERSION, LIBGNOMEUI_MODULE, argc,
			    argv, NULL);

	cache = gnocam_cache_new ();

	result = bonobo_generic_factory_main ("OAFIID:GNOME_GnoCam_Factory",
					      gnocam_factory, NULL);

	g_object_unref (cache);

	return (result);
}
