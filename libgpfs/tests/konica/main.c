#include <config.h>
#include "gpfs-konica.h"

#include <stdio.h>

#include <gphoto2-port.h>
#include <gphoto2-port-log.h>

#if 0
static void
log_func (GPLogLevel level, const char *domain, const char *format,
	  va_list args, void *data)
{
	vprintf (format, args);
	printf ("\n");
}
#endif

int
main (int argc, char **argv)
{
	GPFs *fs;
	GPPort *p;
	GPPortInfoList *il;
	GPPortInfo info;
	int n;
	GPFsBag *b;
	GPFsProp *prop;
	GPFsVal v;

#if 0
	gp_log_add_func (GP_LOG_ALL, log_func, NULL);
#endif

	printf ("Creating list of ports...\n");
	gp_port_info_list_new (&il);
	gp_port_info_list_load (il);
	printf ("... done. Found %i port(s).\n", gp_port_info_list_count (il));

	printf ("Searching first serial port...\n");
	n = gp_port_info_list_lookup_path (il, "serial:/dev/ttyS0");
	gp_port_info_list_get_info (il, n, &info);
	gp_port_info_list_free (il);
	printf ("... done (at position %i).\n", n);

	printf ("Creating port...\n");
	gp_port_new (&p);
	gp_port_set_info (p, info);
	printf ("... done.\n");

	printf ("Creating filesystem...\n");
	fs = gpfs_konica_new (p);
	if (!fs) {
		printf ("... failed.\n");
		return 1;
	}

	printf ("Counting property bags...\n");
	n = gpfs_obj_bag_count (GPFS_OBJ (fs), NULL);
	printf ("... done. Found %i property bag(s).\n", n);

	printf ("Getting first property bag...\n");
	b = gpfs_obj_bag_get (GPFS_OBJ (fs), NULL, 0);
	printf ("... done.\n");

	gpfs_bag_dump (b);

	printf ("Getting first property...\n");
	prop = gpfs_bag_prop_get (b, NULL, 0);
	printf ("... done.\n");

	printf ("Setting new value...\n");
	gpfs_val_init (&v);
	v.t = GPFS_VAL_TYPE_UINT;
	v.v.v_uint = 254;
	gpfs_prop_set_val (prop, NULL, v);
	gpfs_val_clear (&v);
	printf ("... done.\n");

	printf ("Releasing property bag...\n");
	gpfs_obj_unref (GPFS_OBJ (b));
	printf ("... done.\n");

	printf ("Releasing filesystem...\n");
	gpfs_obj_unref (GPFS_OBJ (fs));
	printf ("... done.\n");

	printf ("Releasing port...\n");
	gp_port_free (p);
	printf ("... done.\n");

	return 0;
}
