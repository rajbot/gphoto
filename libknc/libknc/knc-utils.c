#include <config.h>
#include "knc-utils.h"

#include <stdlib.h>
#include <string.h>

#define STX     0x02
#define ETX     0x03
#define ENQ     0x05
#define ACK     0x06
#define XOF     0x11
#define XON     0x13
#define NAK     0x15
#define ETB     0x17
#define ESC     0x1b

void
knc_escape (char *buf, unsigned int *size)
{
	unsigned int i;

	for (i = 0; i < *size; i++) {
		if ((buf[i] == STX) || (buf[i] == ETX) || (buf[i] == ENQ) ||
		    (buf[i] == ACK) || (buf[i] == XOF) || (buf[i] == XON) ||
		    (buf[i] == NAK) || (buf[i] == ETB) || (buf[i] == ESC)) {
			memmove (buf + i + 1, buf + i, *size - i);
			buf[i] = ESC;
			buf[i + 1] = ~buf[i + 1];
			i++; (*size)++;
		}
	}
}

void
knc_unescape (char *buf, unsigned int *size)
{
	unsigned int i;

	for (i = 0; i < *size; i++) {
#ifdef PARANOID_CHECK
		if ((buf[i] == STX) || (buf[i] == ETX) || (buf[i] == ENQ) ||
		    (buf[i] == ACK) || (buf[i] == XOF) || (buf[i] == XON) ||
		    (buf[i] == NAK) || (buf[i] == ETB) || (buf[i] == ESC))
#else
		if ((buf[i] == STX) || (buf[i] == XOF) || (buf[i] == XON))
#endif
			return;
		if (buf[i] == ESC) {
			memmove (buf + i, buf + i + 1, *size - i - 1);
			(*size)--;
			buf[i] = ~buf[i] & 0xff;
#ifdef PARANOID_CHECK
			if ((buf[i] != STX) && (buf[i] != ETX) &&
			    (buf[i] != ENQ) && (buf[i] != ACK) &&
			    (buf[i] != XOF) && (buf[i] != XON) &&
			    (buf[i] != NAK) && (buf[i] != ETB) &&
			    (buf[i] != ESC))
				return;
#endif
		}
	}
}

static struct {
	const char *manufacturer;
	const char *model;
} Devices[] = {
	{"Konica", "Q-EZ"},
	{"Konica", "Q-M100"},
	{"Konica", "Q-M100V"},
	{"Konica", "Q-M200"},
	{"Hewlett Packard", "PhotoSmart"},
	{"Hewlett Packard", "PhotoSmart C20"},
	{"Hewlett Packard", "PhotoSmart C20"},
	{"Hewlett Packard", "PhotoSmart C200"},
};

unsigned int
knc_count_devices (void)
{
	return sizeof (Devices) / sizeof (Devices[0]);
}

const char *
knc_get_device_manufacturer (unsigned int n)
{
	return (n < knc_count_devices ()) ? Devices[n].manufacturer : NULL;
}

const char *
knc_get_device_model (unsigned int n)
{
	return (n < knc_count_devices ()) ? Devices[n].model : NULL;
}
