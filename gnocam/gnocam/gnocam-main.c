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
	GList *cached_cameras;
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
on_camera_destroy (BonoboObject *object, GnoCamMain *m)
{
	m->priv->cached_cameras = g_list_remove (m->priv->cached_cameras,
						 object);
}

static GNOME_GnoCam_Camera
impl_GNOME_GnoCam_Factory_getCamera (PortableServer_Servant servant,
				     CORBA_Environment *ev)
{
	GnoCamMain *gm;
	GnoCamCamera *gc;
	Camera *camera = NULL;

	gm = GNOCAM_MAIN (bonobo_object_from_servant (servant));

	/* Tell libgphoto2 to create and initialize a camera. */
	CR (gp_camera_new (&camera), ev);
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);
	CR (gp_camera_init (camera, NULL), ev);
	if (BONOBO_EX (ev)) {
		gp_camera_unref (camera);
		return (CORBA_OBJECT_NIL);
	}

	/* We have our own idea of what is a camera. */
	gc = gnocam_camera_new (camera, ev);
	gp_camera_unref (camera);
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);

	gm->priv->cached_cameras = g_list_append (gm->priv->cached_cameras, gc);
	g_signal_connect (gc, "destroy", G_CALLBACK (on_camera_destroy), gm);

	return (CORBA_Object_duplicate (BONOBO_OBJREF (gc), ev));
}

static GNOME_GnoCam_Camera
impl_GNOME_GnoCam_Factory_getCameraByModel (
		PortableServer_Servant servant,
		const CORBA_char *model, CORBA_Environment *ev)
{
	GnoCamMain *gm;
	guint i;
	CameraAbilities a;
	CameraAbilitiesList *al = NULL;
	int m;
	Camera *camera;
	GnoCamCamera *gc = NULL;

	gm = GNOCAM_MAIN (bonobo_object_from_servant (servant));

	g_message ("Trying to get a camera for model '%s'...", model);

	/* Cached? */
	for (i = 0; i < g_list_length (gm->priv->cached_cameras); i++) {
		gc = g_list_nth_data (gm->priv->cached_cameras, i);
		gp_camera_get_abilities (gc->camera, &a);
		if (!strcmp (a.model, model))
			break;
	}
	if (i < g_list_length (gm->priv->cached_cameras)) {
		g_message ("Found '%s' in cache.", model);
		bonobo_object_ref (gc);
		return CORBA_Object_duplicate (BONOBO_OBJREF (gc), ev);
	}

	memset (&a, 0, sizeof (CameraAbilities));
	gp_abilities_list_new (&al);
	gp_abilities_list_load (al, NULL);
	m = gp_abilities_list_lookup_model (al, model);
	gp_abilities_list_get_abilities (al, m, &a);
	gp_abilities_list_free (al);

	CR (gp_camera_new (&camera), ev);
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);
	CR (gp_camera_set_abilities (camera, a), ev);
	if (BONOBO_EX (ev)) {
		gp_camera_unref (camera);
		return (CORBA_OBJECT_NIL);
	}

	CR (gp_camera_init (camera, NULL), ev);
	if (BONOBO_EX (ev)) {
		gp_camera_unref (camera); 
		return (CORBA_OBJECT_NIL);
	}

	gc = gnocam_camera_new (camera, ev);
	gp_camera_unref (camera);
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);

	g_message ("Successfully created a camera for model '%s'.", model);

	return (CORBA_Object_duplicate (BONOBO_OBJREF (gc), ev));
}

static GNOME_GnoCam_Camera
impl_GNOME_GnoCam_Factory_getCameraByModelAndPort (
		PortableServer_Servant servant, 
		const CORBA_char *model, const CORBA_char *port,
		CORBA_Environment *ev)
{
	GnoCamMain *gm;
	GnoCamCamera *gc = NULL;
	Camera *camera = NULL;
	guint i;
	CameraAbilities a;
	CameraAbilitiesList *al = NULL;
	GPPortInfoList *il = NULL;
	GPPortInfo info;
	int m, p;

	gm = GNOCAM_MAIN (bonobo_object_from_servant (servant));

	g_message ("Trying to get a camera for model '%s' and port '%s'...",
		   model, port);

	if (!model || !strlen (model) || !port || !strlen (port))
		return GNOME_GnoCam_Factory_getCamera (BONOBO_OBJREF (gm), ev);

	for (i = 0; i < g_list_length (gm->priv->cached_cameras); i++) {
		gc = g_list_nth_data (gm->priv->cached_cameras, i);
		gp_camera_get_abilities (gc->camera, &a);
		if (!strcmp (a.model, model)) {
			gp_camera_get_port_info (gc->camera, &info);
			if (!strcmp (port, info.name) || 
			    !strcmp (port, info.path))
				break;
		}
	}
	if (i < g_list_length (gm->priv->cached_cameras)) {
		bonobo_object_ref (gc);
		g_message ("Found '%s' and '%s' in cache.", model, port);
		return CORBA_Object_duplicate (BONOBO_OBJREF (gc), ev);
	}

	memset (&a, 0, sizeof (CameraAbilities));
	gp_abilities_list_new (&al);
	gp_abilities_list_load (al, NULL);
	m = gp_abilities_list_lookup_model (al, model);
	gp_abilities_list_get_abilities (al, m, &a);
	gp_abilities_list_free (al);

	memset (&info, 0, sizeof (GPPortInfo));
	gp_port_info_list_new (&il);
	gp_port_info_list_load (il);
	p = gp_port_info_list_lookup_name (il, port);
	if (p < 0)
		p = gp_port_info_list_lookup_path (il, port);
	gp_port_info_list_get_info (il, p, &info);
	gp_port_info_list_free (il);

	CR (gp_camera_new (&camera), ev);
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);
	CR (gp_camera_set_abilities (camera, a), ev);
	if (BONOBO_EX (ev)) {
		gp_camera_unref (camera);
		return (CORBA_OBJECT_NIL);
	}
	CR (gp_camera_set_port_info (camera, info), ev);
	if (BONOBO_EX (ev)) {
		gp_camera_unref (camera);
		return (CORBA_OBJECT_NIL);
	}
	CR (gp_camera_init (camera, NULL), ev);
	if (BONOBO_EX (ev)) {
		gp_camera_unref (camera);
		return (CORBA_OBJECT_NIL);
	}

	gc = gnocam_camera_new (camera, ev);
	gp_camera_unref (camera);
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);

	g_message ("Successfully created a camera for '%s' and '%s'.",
		   model, port);

	return (CORBA_Object_duplicate (BONOBO_OBJREF (gc), ev));
}

static void
gnocam_main_finalize (GObject *object)
{
	GnoCamMain *gm = GNOCAM_MAIN (object);

	if (gm->priv) {
		if (gm->priv->cached_cameras) {
			g_warning ("Still %i cameras in cache!",
				   g_list_length (gm->priv->cached_cameras));
			g_list_free (gm->priv->cached_cameras);
			gm->priv->cached_cameras = NULL;
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
	epv->getModelList            = impl_GNOME_GnoCam_Factory_getModelList;
	epv->getPortList             = impl_GNOME_GnoCam_Factory_getPortList;
	epv->getCamera               = impl_GNOME_GnoCam_Factory_getCamera;
	epv->getCameraByModel        = impl_GNOME_GnoCam_Factory_getCameraByModel;
	epv->getCameraByModelAndPort = impl_GNOME_GnoCam_Factory_getCameraByModelAndPort;
}

static void
gnocam_main_init (GnoCamMain *gm)
{
	gm->priv = g_new0 (GnoCamMainPrivate, 1);
}

GnoCamMain *
gnocam_main_new (void)
{
	GnoCamMain *gm;

	gm = g_object_new (GNOCAM_TYPE_MAIN, NULL);

	return (gm);
}

BONOBO_TYPE_FUNC_FULL (GnoCamMain, GNOME_GnoCam_Factory, BONOBO_TYPE_OBJECT,
		       gnocam_main);
