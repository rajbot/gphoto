#include <config.h>
#include "knc-cntrl.h"
#include "knc-utils.h"
#include "knc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#undef DEBUG

#ifdef DEBUG
# include <stdio.h>
# define CR(result) {							\
	KncCntrlRes q = (result);					\
	if (q) {							\
		printf ("   %s/%i: %i: '%s'\n", __FUNCTION__,		\
			__LINE__, q, knc_cntrl_res_name (q));		\
		return q;						\
	}								\
}
#else
# define CR(result) {KncCntrlRes q=(result); if (q) return q;}
#endif

#define DEFAULT_TIMEOUT 500
#define CHECK_INTERVAL 1
#define PING_INTERVAL 5

#define STX     0x02
#define ETX     0x03
#define ENQ     0x05
#define ACK     0x06
#define XOFF    0x11
#define XON     0x13
#define NAK     0x15
#define ETB     0x17
#define ESC     0x1b

#define EOT     0x04

struct _KncCntrl {
	KncCntrlFuncFree func_free;
	void *func_free_data;

	KncCntrlFuncRead  func_read;
	KncCntrlFuncWrite func_write;
	void *func_io_data;

	KncCntrlFuncLog func_log;
	void *func_log_data;

	KncCntrlFuncData func_data;
	void *func_data_data;

	KncCntrlProt prot;

	KncCntrlFuncLock func_lock;
	KncCntrlFuncUnlock func_unlock;
	void *func_lock_data;

	unsigned int ref_count;

	unsigned int cancel : 1;
};

static void
knc_cntrl_log (KncCntrl *c, const char *format, ...)
{
	va_list args;

	if (!c) return;

	va_start (args, format);
	if (c->func_log) c->func_log (format, args, c->func_log_data);
	va_end (args);
}

static KncCntrlRes
knc_cntrl_data (KncCntrl *c, const char *buf, unsigned int size)
{
	if (!c) return KNC_CNTRL_ERR_ILLEGAL_PARAMETER;
	if (!size || !c->func_data) return KNC_CNTRL_OK;

	return c->func_data (buf, size, c->func_data_data);
}

KncCntrlProt
knc_cntrl_prot (KncCntrl *c)
{
	return c ? c->prot : KNC_CNTRL_PROT_NONE;
}

static void
knc_cntrl_free (KncCntrl *c)
{
	if (!c) return;
	if (c->func_free) c->func_free (c, c->func_free_data);
	free (c);
}

void
knc_cntrl_set_func_free (KncCntrl *c, KncCntrlFuncFree f, void *f_data)
{
	if (!c) return;
	c->func_free = f;
	c->func_free_data = f_data;
}

void
knc_cntrl_get_func_free (KncCntrl *c, KncCntrlFuncFree *f, void **f_data)
{
	if (!c) return;
	if (f) *f = c->func_free;
	if (f_data) *f_data = c->func_free_data;
}

static KncCntrlRes
knc_cntrl_write (KncCntrl *c, const char *buf, unsigned int size)
{
	if (!c) return KNC_CNTRL_ERR_ILLEGAL_PARAMETER;

	return c->func_write (buf, size, c->func_io_data);
}

static KncCntrlRes
knc_cntrl_read (KncCntrl *c, char *buf, unsigned int size,
		unsigned int timeout, unsigned int *read)
{
	unsigned int r;

	if (!c) return KNC_CNTRL_ERR_ILLEGAL_PARAMETER;

	CR (c->func_read (buf, size, timeout ? timeout : DEFAULT_TIMEOUT,
			  read ? read : &r, c->func_io_data));
	return KNC_CNTRL_OK;
}

static KncCntrlRes
knc_cntrl_initiate_transfer (KncCntrl *c)
{
	unsigned char b = ENQ;
	unsigned char buf[1024];
	unsigned int read;
	KncCntrlRes r = KNC_CNTRL_OK;

	if (!c) return KNC_CNTRL_ERR_ILLEGAL_PARAMETER;

	while (1) {
		r = knc_cntrl_write (c, &b, 1);         if (r) break;;
		r = knc_cntrl_read (c, &b, 1, 0, NULL); if (r) break;;
		switch (b) {
		case 0x06:
			return KNC_CNTRL_OK;
		case 0x15:
			continue;
		case 0x05:
			r = knc_cntrl_read (c, &b, 1, 0, &read);
			if (r || ((read == 1) && (b == ACK))) break;
			b = 0x15;
			r = knc_cntrl_write (c, &b, 1); if (r) break;;
			continue;
		default:
			r = knc_cntrl_read (c, buf, sizeof (buf), 0, NULL);
			if (r) break;
			continue;
		}
		break;
	}

	return r;
}

static KncCntrlRes
knc_cntrl_read_packet (KncCntrl *c, unsigned char *rb, unsigned int *rbs,
		       unsigned int *last)
{
	unsigned char buf[1024 * 4], b, checksum = 0;
	unsigned int read, i;
	KncCntrlRes r = KNC_CNTRL_OK;

	while (1) {
		r = knc_cntrl_read (c, &b, 1, 5000, NULL); if (r) break;
		if (b != ENQ) {
			knc_cntrl_log (c, "Received unexpected byte "
				       "(0x%02x)!", b);
			r = KNC_CNTRL_ERR;
			break;
		}
		b = ACK; r = knc_cntrl_write (c, &b, 1); if (r) break;
		r = knc_cntrl_read (c, buf, sizeof (buf), 100, &read);
		if (r) break;

		/*
		 * First byte is STX. Everything after that has been excaped
		 * except the ETX or ETB at the end of the packet.
		 */
		if (!read || buf[0] != STX) goto TryAgain;
		memmove (buf, buf + 1, read - 1); read -= 1;
		knc_unescape (buf, &read);
		if (read < 2) goto TryAgain;
		for (i = 0; i < read - 2; i++) checksum += buf[i];

		/*
		 * The next two are the length of the packet not including
		 * ESC quotes, EOT and checksum.
		 */
		if (read < 2) goto TryAgain;
		*rbs = (buf[1] << 8) | buf[0];
		memmove (buf, buf + 2, read - 2); read -= 2;
		if (read != *rbs + 2) {
			knc_cntrl_log (c, "Expected %u byte(s) but got %i!",
				       *rbs + 2, read);
			r = KNC_CNTRL_ERR;
			break;
		}

		/* Then follows data, ETX or ETB, and the checksum. */
		memmove (rb, buf, *rbs);
		memmove (buf, buf + *rbs, read - *rbs); read -= *rbs;
		if ((buf[0] != ETX) && (buf[0] != ETB)) goto TryAgain;
		if (last) *last = (buf[0] == ETX) ? 1 : 0;
#ifdef __GCC__
# warning Fix checksum!
#endif
		/* if (buf[1] != checksum) goto TryAgain; */

		b = ACK; r = knc_cntrl_write (c, &b, 1); if (r) break;
		r = knc_cntrl_read (c, &b, 1, 0, &read); if (r) break;
		if (!read || (b != EOT)) r = KNC_CNTRL_ERR;
		break;

TryAgain:
		b = NAK; r = knc_cntrl_write (c, &b, 1); if (r) break;
	}

	return r;
}

static KncCntrlRes
knc_cntrl_write_packet (KncCntrl *c, const char *buf, unsigned int size)
{
	char b = 0x05;
	KncCntrlRes r = KNC_CNTRL_OK;
	char sb[1024];
	unsigned int sbs, cs = 1, read, i = 0;
	char checksum;

	if (!c) return KNC_CNTRL_ERR_ILLEGAL_PARAMETER;

	/* Construct the packet. */
	memset (sb, 0, sizeof (sb));
	sb[0] = STX;
	sb[1] = size;                 checksum  = sb[1];
	sb[2] = size >> 8;            checksum += sb[2];
	memcpy (sb + 3, buf, size);
	for (i = 0; i < size; i++)    checksum += sb[3 + i];
	knc_escape (3 + sb, &size);
	sb[3 + size] = ETX;           checksum += sb[3 + size];
	sb[3 + size + 1] = checksum;
	knc_escape (3 + sb + size + 1, &cs);
	sbs = 3 + size + 1 + cs;

	/* Write the packet. */
	i = 0;
	while (1) {
		r = knc_cntrl_initiate_transfer (c);       if (r) break;
		r = knc_cntrl_write (c, sb, sbs);          if (r) break;
		r = knc_cntrl_read (c, &b, 1, 0, &read);   if (r) break;
		if (!read) continue;
		switch (b) {
		case ACK:
			b = EOT;
			r = knc_cntrl_write (c, &b, 1);
			break;
		case NAK:
			knc_cntrl_log (c, "Camera rejected packet!");
			if (++i == 3) {
				r = KNC_CNTRL_ERR;
				break;
			}
			continue;
		default:
			knc_cntrl_log (c, "Camera sent unexpeced byte "
				       "(0x%02x)!", b);
			r = KNC_CNTRL_ERR;
			break;
		}
		break;
	}
	return r;
}

KncCntrl *
knc_cntrl_new (KncCntrlFuncRead read, KncCntrlFuncWrite write, void *d)
{
	KncCntrl *c;
	KncCamRes cr;
	KncInfo info;

	c = malloc (sizeof (KncCntrl));
	if (!c) return NULL;
	memset (c, 0, sizeof (KncCntrl));

	c->func_read  = read;
	c->func_write = write;
	c->func_io_data = d;

	c->ref_count = 1;

	/*
	 * I have been told that the Konica QM-200 uses long image IDs.
	 * If image downloading dosn't work with this driver, tell me.
	 */
	if (knc_get_info (c, &cr, &info))
		if (knc_get_info (c, &cr, &info)) return NULL;
	c->prot = KNC_CNTRL_PROT_NONE;

	return c;
}

void
knc_cntrl_ref (KncCntrl *c)
{
	if (c) c->ref_count++;
}

void
knc_cntrl_unref (KncCntrl *c)
{
	if (!c) return;
	if (!--c->ref_count)
		knc_cntrl_free (c);
}

void
knc_cntrl_set_func_data (KncCntrl *c, KncCntrlFuncData f, void *d)
{
	if (!c) return;

	c->func_data = f;
	c->func_data_data = d;
}

static KncCntrlRes
knc_cntrl_lock (KncCntrl *c)
{
	if (!c || !c->func_lock) return KNC_CNTRL_OK;
	return c->func_lock (c->func_lock_data);
}

static void
knc_cntrl_unlock (KncCntrl *c)
{
	if (!c || !c->func_unlock) return;
	c->func_unlock (c->func_lock_data);
}

void
knc_cntrl_set_func_lock (KncCntrl *c, KncCntrlFuncLock f_lock,
			 KncCntrlFuncUnlock f_unlock, void *f_data)
{
	if (!c) return;
	c->func_lock = f_lock;
	c->func_unlock = f_unlock;
	c->func_lock_data = f_data;
}

void
knc_cntrl_get_func_lock (KncCntrl *c, KncCntrlFuncLock *f_lock,
			 KncCntrlFuncUnlock *f_unlock, void **f_data)
{
	if (!c) return;
	if (f_lock) *f_lock = c->func_lock;
	if (f_unlock) *f_unlock = c->func_unlock;
	if (f_data) *f_data = c->func_lock_data;
}

KncCntrlRes
knc_cntrl_transmit (KncCntrl *c, const unsigned char *sb, unsigned int sbs,
		    unsigned char *rb, unsigned int *rbs)
{
	KncCntrlRes r;
	char buf[1024 * 4];
	unsigned int size, last = 0;

	if (!c) return KNC_CNTRL_ERR_ILLEGAL_PARAMETER;

	CR (knc_cntrl_lock (c));
	if ((r = knc_cntrl_write_packet (c, sb, sbs))) {
		knc_cntrl_unlock (c);
		return r;
	}
	if (knc_cmd_data ((sb[1] << 8) | sb[0]))
		while (!r && !last) {
			r = knc_cntrl_read_packet (c, buf, &size, &last);
			if (!r) r = knc_cntrl_data (c, buf, size);
			if (r) knc_cancel (c, NULL, NULL);
		}
	if (r) {knc_cntrl_unlock (c); return r;}

	if ((r = knc_cntrl_read_packet (c, rb, rbs, NULL))) {
		knc_cntrl_unlock (c);
		return r;
	}
	knc_cntrl_unlock (c);

	return KNC_CNTRL_OK;
}

void
knc_cntrl_set_func_log (KncCntrl *c, KncCntrlFuncLog f, void *f_data)
{
	if (!c) return;

	c->func_log = f;
	c->func_log_data = f_data;
}

