/* gnocam-main.c
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
#include <config.h>
#include "gnocam-main.h"

#include <string.h>

#include <bonobo/bonobo-exception.h>

#include <glib.h>

#include "gnocam-util.h"
#include "gnocam-camera.h"

#define PARENT_TYPE BONOBO_OBJECT_TYPE
static BonoboObjectClass *parent_class;

struct _GnoCamMainPrivate
{
	GHashTable *hash_table;
};

static GNOME_GnoCam_Factory_ModelList *
impl_GNOME_GnoCam_Factory_getModelList (PortableServer_Servant servant,
					CORBA_Environment *ev)
{
	GnoCamMain *gm;
	GNOME_GnoCam_Factory_ModelList *list;
	CameraAbilitiesList *al = NULL;
	int n, i;
	CameraAbilities a;

	gm = GNOCAM_MAIN (bonobo_object_from_servant (servant));

	gp_abilities_list_new (&al);
	gp_abilities_list_load (al, NULL);
	n = MAX (0, gp_abilities_list_count (al));
	list = GNOME_GnoCam_Factory_ModelList__alloc ();
	list->_buffer = CORBA_sequence_CORBA_string_allocbuf (n);
	for (i = 0; i < n; i++) {
		if (gp_abilities_list_get_abilities (al, i, &a) >= 0) {
			list->_buffer[list->_length] =
				CORBA_string_dup (a.model);
			list->_length++;
		}
	}
	CORBA_sequence_set_release (list, TRUE);
	gp_abilities_list_free (al);

	return (list);
}

static GNOME_GnoCam_Factory_PortList *
impl_GNOME_GnoCam_Factory_getPortList (PortableServer_Servant servant,
				       CORBA_Environment *ev)
{
	GnoCamMain *gm;
	GNOME_GnoCam_Factory_PortList *list;
	GPPortInfoList *il = NULL;
	GPPortInfo info;
	int n, i;

	gm = GNOCAM_MAIN (bonobo_object_from_servant (servant));

	gp_port_info_list_new (&il);
	gp_port_info_list_load (il);
	n = MAX (0, gp_port_info_list_count (il));
	g_message ("%i port(s) available.", n);

	list = GNOME_GnoCam_Factory_PortList__alloc ();
	list->_buffer = CORBA_sequence_CORBA_string_allocbuf (n);
	for (i = 0; i < n; i++) {
		if (gp_port_info_list_get_info (il, i, &info) >= 0) {
			list->_buffer[list->_length] =
				CORBA_string_dup (info.name);
			list->_length++;
		}
	}
	CORBA_sequence_set_release (list, TRUE);
	gp_port_info_list_free (il);

	return (list);
}

static void
initialize_camera (Camera *camera, const gchar *name,
		   CORBA_Environment *ev)
{
	gint i;
	CameraAbilitiesList *al;
	CameraAbilities a;

	g_message ("Looking for %s...", name);
	gp_abilities_list_new (&al);
	gp_abilities_list_load (al, NULL);
	for (i = 0; i < gp_abilities_list_count (al); i++) {
		gp_abilities_list_get_abilities (al, i, &a);
		if (!strcmp (a.model, name))
			break;
	}
	if (i >= gp_abilities_list_count (al)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_GNOME_GnoCam_Factory_NotFound, NULL);
		return;
	}
	gp_abilities_list_free (al);

	g_message ("Trying to initialize %s...", name);
	CR (gp_camera_set_abilities (camera, a), ev);
	CR (gp_camera_init (camera, NULL), ev);
}

static GNOME_GnoCam_Camera
impl_GNOME_GnoCam_Factory_getCamera (PortableServer_Servant servant,
				     CORBA_Environment *ev)
{
	GnoCamMain *gm;
	GnoCamCamera *gc;
	Camera *camera = NULL;

	g_message ("impl_GNOME_GnoCam_getCamera");

	gm = GNOCAM_MAIN (bonobo_object_from_servant (servant));
	CR (gp_camera_new (&camera), ev);
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);

	/* Ask! */
	g_warning ("Not implemented!");
	gp_camera_unref (camera);
	return (CORBA_OBJECT_NIL);
#if 0
			GnomeDialog *selector;
			const gchar *name;
			gint button;
	
			selector = gnocam_camera_selector_new ();
			do {
				button = gnome_dialog_run (selector);
				if (button == 1) {
					/* Cancel */
					gnome_dialog_close (selector);
					CORBA_exception_set (ev,
						CORBA_USER_EXCEPTION,
						ex_GNOME_Cancelled, NULL);
					gp_camera_unref (camera);
					return (CORBA_OBJECT_NIL);
				} else if (button == 0) {
					break;
				} else {
					g_warning ("Unhandled button: %i",
						   button);
				}
			} while (TRUE);
	
			name = gnocam_camera_selector_get_name (
					GNOCAM_CAMERA_SELECTOR (selector));
			gnome_dialog_close (selector);
			while (gtk_events_pending ())
				gtk_main_iteration ();

			/* It can well be that the user clicked autodetect */
			if (gconf_client_get_bool (gm->priv->client,
						"/apps/" PACKAGE "/autodetect",
						NULL)) {
				CR (gp_camera_init (camera, NULL), ev);
				if (BONOBO_EX (ev)) {
					gp_camera_unref (camera);
					return (CORBA_OBJECT_NIL);
				}
			} else if (!name) {
				CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					ex_GNOME_GnoCam_Factory_NotFound,
						     NULL);
				gp_camera_unref (camera);
				return (CORBA_OBJECT_NIL);
			} else {
				initialize_camera (camera, name, ev);
				if (BONOBO_EX (ev)) {
					gp_camera_unref (camera);
					return (CORBA_OBJECT_NIL);
				}
			}
#endif

	gc = gnocam_camera_new (camera, ev);
	gp_camera_unref (camera);
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);

	return (CORBA_Object_duplicate (BONOBO_OBJREF (gc), ev));
}

static GNOME_GnoCam_Camera
impl_GNOME_GnoCam_Factory_getCameraByModel (PortableServer_Servant servant, 
		const CORBA_char *model, CORBA_Environment *ev)
{
	GnoCamMain *gm;
	GnoCamCamera *gc;
	Camera *camera = NULL;

	g_message ("impl_GNOME_GnoCam_getCamera: %s", model);

	gm = GNOCAM_MAIN (bonobo_object_from_servant (servant));

	if (!model)
		return GNOME_GnoCam_Factory_getCamera (BONOBO_OBJREF (gm), ev);

	camera = g_hash_table_lookup (gm->priv->hash_table, model);
	if (!camera) {
		CR (gp_camera_new (&camera), ev);
		if (BONOBO_EX (ev))
			return (CORBA_OBJECT_NIL);
		initialize_camera (camera, model, ev);
		if (BONOBO_EX (ev)) {
			gp_camera_unref (camera);
			return (CORBA_OBJECT_NIL);
		}
		g_hash_table_insert (gm->priv->hash_table,
				     g_strdup (model), camera);
	}

	gc = gnocam_camera_new (camera, ev);
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);

	return (CORBA_Object_duplicate (BONOBO_OBJREF (gc), ev));
}

static gboolean
foreach_func (gpointer key, gpointer value, gpointer user_data)
{
	Camera *camera;

	camera = value;
	gp_camera_unref (camera);

	return (TRUE);
}

static void
gnocam_main_finalize (GObject *object)
{
	GnoCamMain *gm = GNOCAM_MAIN (object);

	if (gm->priv) {
		if (gm->priv->hash_table) {
			g_hash_table_foreach_remove (gm->priv->hash_table,
						     foreach_func, NULL);
			g_hash_table_destroy (gm->priv->hash_table);
			gm->priv->hash_table = NULL;
		}

		g_free (gm->priv);
		gm->priv = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_main_class_init (GnoCamMainClass *klass)
{
	GObjectClass *object_class;
	POA_GNOME_GnoCam_Factory__epv *epv; 

	parent_class = g_type_class_peek_parent (klass);

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gnocam_main_finalize;

	epv = &klass->epv;
	epv->getModelList     = impl_GNOME_GnoCam_Factory_getModelList;
	epv->getPortList      = impl_GNOME_GnoCam_Factory_getPortList;
	epv->getCamera        = impl_GNOME_GnoCam_Factory_getCamera;
	epv->getCameraByModel = impl_GNOME_GnoCam_Factory_getCameraByModel;
}

static void
gnocam_main_init (GnoCamMain *gm)
{
	gm->priv = g_new0 (GnoCamMainPrivate, 1);

	gm->priv->hash_table = g_hash_table_new (g_str_hash, g_str_equal);
}

GnoCamMain *
gnocam_main_new (void)
{
	GnoCamMain *gm;

	g_message ("Creating a GnoCamMain...");

	gm = g_object_new (GNOCAM_TYPE_MAIN, NULL);

	return (gm);
}

BONOBO_TYPE_FUNC_FULL (GnoCamMain, GNOME_GnoCam_Factory, BONOBO_TYPE_OBJECT,
		       gnocam_main);
