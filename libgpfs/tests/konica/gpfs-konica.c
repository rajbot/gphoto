#include <config.h>
#include "gpfs-konica.h"

#include <libgpfs/gpfs-i18n.h>

#include "konica.h"

enum {
	PROP_SELF_TIMER_TIME,
	PROP_BEEP,
	PROP_SLIDE_SHOW_INTERVAL,
	PROP_AUTO_OFF_TIME
};

/* Property */
static void
f_prop_set_val (GPFsProp *p, GPFsErr *e, GPFsVal v, void *d)
{
	GPPort *port = (GPPort *) d;

	switch (gpfs_obj_get_id (GPFS_OBJ (p))) {
	case PROP_SLIDE_SHOW_INTERVAL:
		k_set_preference (port, e, K_PREFERENCE_SLIDE_SHOW_INTERVAL,
				  v.v.v_uint);
		break;
	case PROP_SELF_TIMER_TIME:
		k_set_preference (port, e, K_PREFERENCE_SELF_TIMER_TIME,
				  v.v.v_uint);
		break;
	case PROP_BEEP:
		k_set_preference (port, e, K_PREFERENCE_BEEP, v.v.v_bool);
		break;
	case PROP_AUTO_OFF_TIME:
		k_set_preference (port, e, K_PREFERENCE_AUTO_OFF_TIME,
				  v.v.v_uint);
		break;
	}
}

/* Property bag */

static GPFsBag *
f_fs_bag_get (GPFsObj *o, GPFsErr *e, unsigned int n, void *d)
{
	KPreferences prefs;
	GPFsProp *p = NULL;
	GPPort *port = (GPPort *) d;
	GPFsVal v;
	GPFsBag *b;

	if (n > 0) {
		gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS, _("There is no "
			"property bag %i."), n);
		return NULL;
	}

	if (k_get_preferences (port, e, &prefs) < 0)
		return NULL;

	b = gpfs_bag_new ();

	v.t = GPFS_VAL_TYPE_UINT;
	v.v.v_uint = prefs.shutoff_time;
	p = gpfs_prop_new (v);
	gpfs_obj_set_id (GPFS_OBJ (p), PROP_AUTO_OFF_TIME);
	gpfs_obj_set_name_impl (GPFS_OBJ (p), _("Shutoff time"));
	gpfs_obj_set_descr_impl (GPFS_OBJ (p),
		_("The time after which the camera shuts itself "
		  "off automatically."));
	gpfs_bag_prop_add_impl (b, p);
	gpfs_prop_set_func_set_val (p, f_prop_set_val, d);
	p->t = GPFS_ALT_TYPE_RANGE;
	p->alt.range.min = 0; p->alt.range.max = 255; p->alt.range.incr = 1;

	v.t = GPFS_VAL_TYPE_UINT; 
	v.v.v_uint = prefs.self_timer_time;
	p = gpfs_prop_new (v);
	gpfs_obj_set_id (GPFS_OBJ (p), PROP_SELF_TIMER_TIME);
	gpfs_obj_set_name_impl (GPFS_OBJ (p), _("Self timer time"));
	gpfs_obj_set_descr_impl (GPFS_OBJ (p),
		_("The time after which an image is taken."));
	gpfs_bag_prop_add_impl (b, p);
	gpfs_prop_set_func_set_val (p, f_prop_set_val, d);
	p->t = GPFS_ALT_TYPE_RANGE;
	p->alt.range.min = 0; p->alt.range.max = 255; p->alt.range.incr = 1;

	v.t = GPFS_VAL_TYPE_BOOL;
	v.v.v_bool = prefs.beep;
	p = gpfs_prop_new (v);
	gpfs_obj_set_id (GPFS_OBJ (p), PROP_BEEP);
	gpfs_obj_set_name_impl (GPFS_OBJ (p), _("Beep"));
	gpfs_obj_set_descr_impl (GPFS_OBJ (p),
		_("Defines whether the camera beeps when capturing images."));
	gpfs_bag_prop_add_impl (b, p);
	gpfs_prop_set_func_set_val (p, f_prop_set_val, d);

	v.t = GPFS_VAL_TYPE_UINT;
	v.v.v_uint = prefs.slide_show_interval;
	p = gpfs_prop_new (v);
	gpfs_obj_set_id (GPFS_OBJ (p), PROP_SLIDE_SHOW_INTERVAL);
	gpfs_obj_set_name_impl (GPFS_OBJ (p), _("Slide show interval"));
	gpfs_obj_set_descr_impl (GPFS_OBJ (p),
		_("Duration between two slides during the slide show."));
	gpfs_bag_prop_add_impl (b, p);
	gpfs_prop_set_func_set_val (p, f_prop_set_val, d);
	p->t = GPFS_ALT_TYPE_RANGE;
	p->alt.range.min = 0; p->alt.range.max = 255; p->alt.range.incr = 1;

	return b;
}

static unsigned int
f_fs_bag_count (GPFsObj *o, GPFsErr *e, void *d)
{
	return 1;
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
