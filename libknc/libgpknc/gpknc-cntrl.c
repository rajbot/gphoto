#include <config.h>
#include "gpknc-cntrl.h"

#include <gphoto2-port.h>
#include <gphoto2-port-result.h>

static KncCntrlRes
func_read (unsigned char *buf, unsigned int size, unsigned int timeout,
	   unsigned int *read, void *data)
{
	GPPort *p = data;
	int r = GP_OK;
	
	gp_port_set_timeout (p, timeout);
	for (*read = 0; *read < size; (*read)++) {
		r = gp_port_read (p, &buf[*read], 1);
		if (r < 0) break;
	}
	if ((*read == 0) && (r < 0)) return KNC_CNTRL_ERR;
	return KNC_CNTRL_OK;
}

static KncCntrlRes
func_write (const unsigned char *buf, unsigned int size, void *data)
{
	GPPort *p = data;
	
	if (gp_port_write (p, buf, size) < 0) return KNC_CNTRL_ERR;
	return KNC_CNTRL_OK;
}

static void
func_free (KncCntrl *c, void *data)
{
	gp_port_free ((GPPort *) data);
}

KncCntrl *
gpknc_cntrl_new_from_path (const char *path)
{
	GPPort *p = NULL;
	KncCntrl *c;
	GPPortInfoList *il;
	GPPortInfo info;
	int n;

	gp_port_info_list_new (&il);
	gp_port_info_list_load (il);
	n = gp_port_info_list_lookup_path (il, path);
	gp_port_info_list_get_info (il, n, &info);
	gp_port_info_list_free (il);
	gp_port_new (&p);
	gp_port_set_info (p, info);

	c = gpknc_cntrl_new_from_port (p);
	if (!c) {
		gp_port_free (p);
		return NULL;
	}
	knc_cntrl_set_func_free (c, func_free, p);
	return c;
}

KncCntrl *
gpknc_cntrl_new_from_port (GPPort *p)
{
	GPPortSettings s;
	KncCntrl *c;

	gp_port_get_settings (p, &s);
	s.serial.speed = 9600;
	s.serial.parity = GP_PORT_SERIAL_PARITY_OFF;
	gp_port_set_settings (p, s);
	c = knc_cntrl_new (func_read, func_write, p);
	if (!c) {
		s.serial.speed = 115200;
		gp_port_set_settings (p, s);
		c = knc_cntrl_new (func_read, func_write, p);
		if (!c) return NULL;
	}
	return c;
}

GPPort *
gpknc_cntrl_get_port (KncCntrl *c)
{
	GPPort *p;

	knc_cntrl_get_func_free (c, NULL, (void **) &p);

	return p;
}
