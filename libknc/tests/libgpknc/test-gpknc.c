#include <config.h>

#include <libknc/knc-cntrl.h>
#include <libknc/knc.h>

#include <libgpknc/gpknc-cntrl.h>

#include <stdio.h>
#include <stdlib.h>

#include <gphoto2-port.h>
#include <gphoto2-port-log.h>
#include <gphoto2-port-result.h>

static KncCntrlRes
test_data (const unsigned char *buf, unsigned int size, void *data)
{
	return KNC_CNTRL_OK;
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
	GPPort *p;
	KncCntrl *c;
	KncCamRes r;
	KncInfo ki;
	KncImageInfo kii;
	KncBitRate br;
	KncBitFlag bf;
	GPPortSettings s;

	printf ("Connecting to camera...\n");
	c = gpknc_cntrl_new_from_path ("serial:/dev/ttyS0");
	if (!c) {
		printf ("... failed.\n");
		return 1;
	}
	knc_cntrl_set_func_data (c, test_data, NULL);
	knc_cntrl_set_func_log  (c, test_log , NULL);
	printf ("... done.\n");

	printf ("Setting speed to 115200...\n");
	p = gpknc_cntrl_get_port (c);
	br = KNC_BIT_RATE_115200; bf = KNC_BIT_FLAG_8_BITS;
	CR (knc_set_io_pref (c, NULL, &br, &bf));
	gp_port_get_settings (p, &s);
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

	return 0;
}
