#include <config.h>
#include <libknc/knc-cntrl.h>
#include <libknc/knc.h>

#include <stdio.h>
#include <stdlib.h>

#include <gphoto2-port.h>
#include <gphoto2-port-log.h>
#include <gphoto2-port-result.h>

#define LOG_DATA

static void
log_func (GPLogLevel level, const char *domain, const char *format,
	  va_list args, void *d)
{
	vprintf (format, args);
	printf ("\n");
}

static KncCntrlRes
test_read (unsigned char *buf, unsigned int size, unsigned int timeout,
	   unsigned int *read, void *data)
{
	GPPort *p = data;
	int r = GP_OK;
#ifdef LOG_DATA
	unsigned int i;
#endif

	gp_port_set_timeout (p, timeout);

	for (*read = 0; *read < size; (*read)++) {
		r = gp_port_read (p, &buf[*read], 1);
		if (r < 0)
			break;
	}
	if ((*read == 0) && (r < 0))
		return KNC_CNTRL_RES_ERR;

#ifdef LOG_DATA
	printf ("%i byte(s) read:\n", *read);
	for (i = 0; i < *read; i++) printf ("%02x ", buf[i]); printf ("\n");
#endif

	return KNC_CNTRL_RES_OK;
}

static KncCntrlRes
test_write (const unsigned char *buf, unsigned int size, void *data)
{
	GPPort *p = data;
#ifdef LOG_DATA
	unsigned int i;
#endif

	if (gp_port_write (p, buf, size) < 0)
		return KNC_CNTRL_RES_ERR;
#ifdef LOG_DATA
	printf ("%i byte(s) written:\n", size);
	for (i = 0; i < size; i++) printf ("%02x ", buf[i]); printf ("\n");
#endif

	return KNC_CNTRL_RES_OK;
}

static KncCntrlRes
test_data (const unsigned char *buf, unsigned int size, void *data)
{
	return KNC_CNTRL_RES_OK;
}

static void
test_log (const char *format, va_list args, void *data)
{
	vprintf (format, args);
	printf ("\n");
}

#define CR(result) {					\
	KncCntrlRes res = (result);			\
	if (res) {					\
		printf ("### Error %i: '%s'\n", res,	\
			knc_cntrl_res_name (res));	\
		return 1;				\
	}						\
}

int
main (int argc, char **argv)
{
	GPPortInfoList *il;
	GPPortInfo info;
	int n;
	GPPort *p;
	GPPortSettings s;
	KncCntrl *c;
	KncCamRes r;
	KncInfo ki;
	KncImageInfo kii;
	KncBitRate br;
	KncBitFlag bf;

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

	gp_port_get_settings (p, &s);
	s.serial.speed = 9600;
	s.serial.parity = GP_PORT_SERIAL_PARITY_OFF;
	gp_port_set_settings (p, s);

	if (argc > 1) gp_log_add_func (GP_LOG_ALL, log_func, NULL);

	printf ("Connecting to camera...\n");
	c = knc_cntrl_new (test_read, test_write, p);
	if (!c) {
		s.serial.speed = 115200;
		gp_port_set_settings (p, s);
		c = knc_cntrl_new (test_read, test_write, p);
		if (!c) {
			printf ("... failed.\n");
			return 1;
		}
	};
	knc_cntrl_set_func_data (c, test_data, NULL);
	knc_cntrl_set_func_log  (c, test_log , NULL);
	printf ("... done.\n");

	printf ("Setting speed to 115200...\n");
	br = KNC_BIT_RATE_115200; bf = KNC_BIT_FLAG_8_BITS;
	CR (knc_set_io_pref (c, NULL, &br, &bf));
	s.serial.speed = 115200;
	gp_port_set_settings (p, s);

	printf ("Requesting information about the camera...\n");
	CR (knc_get_info (c, &r, &ki));
	if (r) printf ("### Error %i: '%s'!\n", r, knc_cam_res_name (r));
	else {
		printf (" - Model '%s'\n", ki.model);
		printf (" - Serial number '%s'\n", ki.serial_number);
		printf (" - Name '%s'\n", ki.name);
		printf (" - Manufacturer '%s'\n", ki.manufacturer);
	}

	printf ("Capturing preview...\n");
	CR (knc_get_preview (c, &r, KNC_PREVIEW_NO));

	printf ("Requesting information about the first image...\n");
	CR (knc_get_image_info (c, &r, 1, &kii));
	if (r) printf ("### Error 0x%04x: '%s!\n", r, knc_cam_res_name (r));
	else {
		printf (" - ID %li\n", kii.id);
		printf (" - Size %i\n", kii.size);
		printf (" - Protected: %s\n", kii.prot ? "yes" : "no");
	}

	printf ("Downloading preview of first image...\n");
	CR (knc_get_image (c, NULL, kii.id, KNC_SOURCE_CARD, KNC_IMAGE_THUMB));
	knc_cntrl_unref (c);
	gp_port_free (p);

	return 0;
}
