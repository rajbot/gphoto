#include <config.h>
#include "gpfs-konica.h"

#include <libgpfs/gpfs-i18n.h>

#include "konica.h"

/* Property bag */

static unsigned int
f_fs_b_prop_count (GPFsBag *o, GPFsErr *e, void *d)
{
	return 4;
}

static GPFsProp *
f_fs_b_prop_get (GPFsBag *o, GPFsErr *e, unsigned int n, void *d)
{
	KPreferences prefs;
	GPFsProp *p = NULL;
	GPPort *port = (GPPort *) d;
	GPFsVal v;

	if (k_get_preferences (port, e, &prefs) < 0)
		return NULL;

	switch (n) {
	case 0:
		v.t = GPFS_VAL_TYPE_UINT;
		v.v.v_uint = prefs.shutoff_time;
		p = gpfs_prop_new (1, _("Shutoff time"),
			_("The time after which the camera shuts itself "
			  "off automatically."), v);
		p->t = GPFS_ALT_TYPE_RANGE;
		p->alt.range.min = 0;
		p->alt.range.max = 255;
		p->alt.range.incr = 1;
		break;
	case 1:
		v.t = GPFS_VAL_TYPE_UINT; 
		v.v.v_uint = prefs.self_timer_time;
		p = gpfs_prop_new (2, _("Self timer time"),
			_("The time after which an image is taken."), v);
		break;
	case 2:
		v.t = GPFS_VAL_TYPE_UINT;
		v.v.v_uint = prefs.beep;
		p = gpfs_prop_new (3, _("Beep"),
			_("Defines whether the camera beeps when capturing "
			  "images."), v);
		break;
	case 3:
		v.t = GPFS_VAL_TYPE_UINT;
		v.v.v_uint = prefs.slide_show_interval;
		p = gpfs_prop_new (4,
			_("Slide show interval"),
			_("Duration between two slides during the slide "
			  "show."), v);
		break;
	default:
		gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS, _("There is no "
			"property %i."), n + 1);
		return NULL;
	}

	return p;
}

/* Filesystem */

static unsigned int
f_fs_bag_count (GPFsObj *o, GPFsErr *e, void *d)
{
	return 1;
}

static GPFsBag *
f_fs_bag_get (GPFsObj *o, GPFsErr *e, unsigned int n, void *d)
{
	GPFsBag *b = gpfs_bag_new ();

	gpfs_bag_set_func_prop_count (b, f_fs_b_prop_count, d);
	gpfs_bag_set_func_prop_get   (b, f_fs_b_prop_get  , d);

	return b;
}

GPFs *
gpfs_konica_new (GPPort *p)
{
	GPFs *fs;
	GPPortSettings s;
	unsigned int n;

	gp_port_get_settings (p, &s);
	s.serial.bits = 8;
	s.serial.parity = 0;
	s.serial.stopbits = 1;
	s.serial.speed = 9600;
	gp_port_set_settings (p, s);
	for (n = 0; n < 5; n++) if (k_init (p, NULL) >= 0) break;
	if (n == 5) return NULL;

	fs = gpfs_new ();

	gpfs_obj_set_func_bag_count (GPFS_OBJ (fs), f_fs_bag_count, p);
	gpfs_obj_set_func_bag_get   (GPFS_OBJ (fs), f_fs_bag_get  , p);

	return fs;
}
