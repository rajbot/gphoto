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

static GNOME_GnoCam_ModelList *
impl_GNOME_GnoCam_getModelList (PortableServer_Servant servant,
					CORBA_Environment *ev)
{
	GnoCamMain *gm;
	GNOME_GnoCam_ModelList *list;
	CameraAbilitiesList *al = NULL;
	int n, i;
	CameraAbilities a;

	gm = GNOCAM_MAIN (bonobo_object_from_servant (servant));

	gp_abilities_list_new (&al);
	gp_abilities_list_load (al, NULL);
	n = MAX (0, gp_abilities_list_count (al));
	list = GNOME_GnoCam_ModelList__alloc ();
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

static GNOME_GnoCam_PortList *
impl_GNOME_GnoCam_getPortList (PortableServer_Servant servant,
				       CORBA_Environment *ev)
{
	GnoCamMain *gm;
	GNOME_GnoCam_PortList *list;
	GPPortInfoList *il = NULL;
	GPPortInfo info;
	int n, i;

	gm = GNOCAM_MAIN (bonobo_object_from_servant (servant));

	gp_port_info_list_new (&il);
	gp_port_info_list_load (il);
	n = MAX (0, gp_port_info_list_count (il));

	list = GNOME_GnoCam_PortList__alloc ();
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
	g_message ("A camera got destroyed.");

	m->priv->cached_cameras = g_list_remove (m->priv->cached_cameras,
						 object);
}

static GnoCamCamera *
gnocam_main_get_camera (GnoCamMain *gm, const gchar *model, const gchar *port,
		        CORBA_Environment *ev)
{
	guint i;
	Camera *camera;
	GnoCamCamera *gc = NULL;

	g_message ("Trying to get a camera for model '%s' (port '%s')...",
		   model, port);

	g_return_val_if_fail (GNOCAM_IS_MAIN (gm), NULL);

	/* Cached? */
	if (model && strlen (model)) {

		g_message ("Looking for model '%s' in cache (%i entries)...",
			   model, g_list_length (gm->priv->cached_cameras));
	        for (i = 0; i < g_list_length (gm->priv->cached_cameras); i++) {
			CameraAbilities a;

			memset (&a, 0, sizeof (CameraAbilities));
	                gc = g_list_nth_data (gm->priv->cached_cameras, i);
	                gp_camera_get_abilities (gc->camera, &a);
	                if (!strcmp (a.model, model)) {
				if (port && strlen (port)) {
				    GPPortInfo info;

				    memset (&info, 0, sizeof (GPPortInfo));
				    gp_camera_get_port_info (gc->camera, &info);
				    if (!strcmp (port, info.name) || 
				        !strcmp (port, info.path))
					break;
				} else
				    break;
			}
	        }
        	if (i < g_list_length (gm->priv->cached_cameras)) {
        	        bonobo_object_ref (gc);
        	        return gc;
        	}
	}

	CR (gp_camera_new (&camera), ev);
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);

	if (model && strlen (model)) {
		CameraAbilities a;
		CameraAbilitiesList *al = NULL;
		int m;

	        memset (&a, 0, sizeof (CameraAbilities));
	        gp_abilities_list_new (&al);
	        gp_abilities_list_load (al, NULL);
	        m = gp_abilities_list_lookup_model (al, model);
	        gp_abilities_list_get_abilities (al, m, &a);
	        gp_abilities_list_free (al);
		CR (gp_camera_set_abilities (camera, a), ev);
		if (BONOBO_EX (ev)) {
			gp_camera_unref (camera);
			return NULL;
		}
	}

	if (port && strlen (port)) {
		GPPortInfo info;
		GPPortInfoList *il = NULL;
		int p;

		memset (&info, 0, sizeof (GPPortInfo));
		gp_port_info_list_new (&il);
		gp_port_info_list_load (il);
		p = gp_port_info_list_lookup_name (il, port);
		if (p < 0)
			p = gp_port_info_list_lookup_path (il, port);
		gp_port_info_list_get_info (il, p, &info);
		gp_port_info_list_free (il);
		CR (gp_camera_set_port_info (camera, info), ev); 
		if (BONOBO_EX (ev)) { 
			gp_camera_unref (camera);
			return (CORBA_OBJECT_NIL);
		}
	}

        CR (gp_camera_init (camera, NULL), ev);
        if (BONOBO_EX (ev)) {
                gp_camera_unref (camera); 
                return NULL;
        }

        gc = gnocam_camera_new (camera, ev);
        gp_camera_unref (camera);
        if (BONOBO_EX (ev))
                return NULL;

	gm->priv->cached_cameras = g_list_append (gm->priv->cached_cameras, gc);        g_signal_connect (gc, "destroy", G_CALLBACK (on_camera_destroy), gm);

        g_message ("Successfully created a camera.");

	return gc;
}

typedef struct _IdleData IdleData;
struct _IdleData {
	GnoCamMain *gm;
	Bonobo_Listener listener;
	gchar *model;
	gchar *port;
};

static void
idle_data_destroy (gpointer data)
{
	IdleData *d = data;

	bonobo_object_unref (d->gm);
	g_free (d->model);
	g_free (d->port);
	g_free (d);
}

static gboolean
get_camera_idle (gpointer data)
{
	IdleData *d = data;
	GnoCamCamera *gc;
	CORBA_Environment ev;
	CORBA_any any;

	CORBA_exception_init (&ev);

	gc = gnocam_main_get_camera (d->gm, d->model, d->port, &ev);
	if (BONOBO_EX (&ev)) {
		GNOME_GnoCam_ErrorCode e = GNOME_GnoCam_ERROR;
		any._type = TC_GNOME_GnoCam_ErrorCode;
		any._value = &e;
	} else {
		GNOME_Camera c = CORBA_Object_duplicate (BONOBO_OBJREF (gc),
							 &ev);
		any._type = TC_GNOME_Camera;
		any._value = &c;
	}

	g_message ("Notifying...");
	Bonobo_Listener_event (d->listener, "result", &any, &ev);
	g_message ("Notified.");

	CORBA_exception_free (&ev);
	
	return (FALSE);
};

static GNOME_Camera
impl_GNOME_GnoCam_getCamera (PortableServer_Servant servant,
			     CORBA_Environment *ev)
{
	GnoCamMain *gm;
	GnoCamCamera *gc;

	gm = GNOCAM_MAIN (bonobo_object_from_servant (servant));

	gc = gnocam_main_get_camera (gm, NULL, NULL, ev);

	return (CORBA_Object_duplicate (BONOBO_OBJREF (gc), ev));
}

static void
impl_GNOME_GnoCam_getCameraByModel (
		PortableServer_Servant servant,
		const Bonobo_Listener listener, const CORBA_char *model,
		CORBA_Environment *ev)
{
	IdleData *d;

	d = g_new0 (IdleData, 1);
	d->model = g_strdup (model);
	d->gm = GNOCAM_MAIN (bonobo_object_from_servant (servant));
	bonobo_object_ref (d->gm);
	d->listener = CORBA_Object_duplicate (listener, NULL);

	g_message ("Adding idle function...");
	g_idle_add_full (0, get_camera_idle, d, idle_data_destroy);
}

static GNOME_Camera
impl_GNOME_GnoCam_getCameraByModelAndPort (
		PortableServer_Servant servant, 
		const CORBA_char *model, const CORBA_char *port,
		CORBA_Environment *ev)
{
	GnoCamMain *gm;
	GnoCamCamera *gc = NULL;

	gm = GNOCAM_MAIN (bonobo_object_from_servant (servant));

	gc = gnocam_main_get_camera (gm, model, port, ev);
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);

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
gnocam_main_class_init (gpointer g_class, gpointer class_data)
{
	GnoCamMainClass *gnocam_class;
	GObjectClass *gobject_class;
	POA_GNOME_GnoCam__epv *epv; 

	parent_class = g_type_class_peek_parent (g_class);

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gnocam_main_finalize;

	gnocam_class = GNOCAM_MAIN_CLASS (g_class);
	epv = &gnocam_class->epv;

	/* Sync interface */
	epv->getModelList           = impl_GNOME_GnoCam_getModelList;
	epv->getPortList            = impl_GNOME_GnoCam_getPortList;
	epv->getCamera              = impl_GNOME_GnoCam_getCamera;
	epv->getCameraByModelAndPort= impl_GNOME_GnoCam_getCameraByModelAndPort;

	epv->getCameraByModel  = impl_GNOME_GnoCam_getCameraByModel;
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

BONOBO_TYPE_FUNC_FULL (GnoCamMain, GNOME_GnoCam, BONOBO_TYPE_OBJECT,
		       gnocam_main);
