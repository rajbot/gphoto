/* knc.c
 *
 * Copyright (C) 2001 Lutz Müller
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
#include "knc.h"
#include "knc-i18n.h"

#include <stdlib.h>
#include <string.h>

#define DEFAULT_TIMEOUT 500

#define CR(result) {KncCntrlRes r = (result); if (r) return r;}
#define CS(s1,s2) {if ((s1) != (s2)) return KNC_CNTRL_RES_ERR;}
#define CCR(r,b,s) {							\
	if ((s) < 4) return KNC_CNTRL_RES_ERR;				\
	if (!(r) && (((b)[3] << 8) | (b)[2])) return KNC_CNTRL_RES_ERR;	\
	if (r) {							\
		*(r) = ((b)[3] << 8) | (b)[2];				\
		if (*(r)) return KNC_CNTRL_RES_OK;			\
	}								\
}

KncCntrlRes
knc_erase_image (KncCntrl *c, KncCamRes *r, unsigned long n, KncSource s)
{
        unsigned char sb[] = {0, 0x80, 0, 0, s, s >> 8, 0, 0, 0, 0};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);
	KncCntrlProt prot = knc_cntrl_prot (c);

	if (prot & KNC_CNTRL_PROT_LONG_ID) {
		sb[6] = n >> 16;
		sb[7] = n >> 24;
		sb[8] = n;
		sb[9] = n >> 8;
		CR (knc_cntrl_transmit (c, sb, 10, rb, &rbs));
	} else {
		sb[6] = n;
		sb[7] = n >> 8;
		CR (knc_cntrl_transmit (c, sb, 8, rb, &rbs));
	}
	CCR (r,rb,rbs);
	CS (rbs, 4);
        return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_format (KncCntrl *c, KncCamRes *r, KncSource s)
{
        const unsigned char sb[] = {0x10, 0x80, 0, 0, s, s >> 8};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 4);
	return KNC_CNTRL_RES_OK;
}


KncCntrlRes
knc_erase_all (KncCntrl *c, KncCamRes *r, KncSource s, unsigned int *n)
{
	const unsigned char sb[] = {0x20, 0x80, 0, 0, s, s >> 8};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 6);
	if (n) *n = (rb[5] << 8) | rb[4];
	return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_set_prot (KncCntrl *c, KncCamRes *r, unsigned long n, KncSource s,
	      KncProt p)
{
        unsigned char sb[] = {0x30, 0x80, 0, 0, s, s >> 8, 0, 0, 0, 0, 0, 0, 0};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);
	KncCntrlProt prot = knc_cntrl_prot (c);

        if (prot & KNC_CNTRL_PROT_LONG_ID) {
		sb[ 6] = n >> 16;
		sb[ 7] = n >> 24;
		sb[ 8] = n;
		sb[ 9] = n >> 8;
		sb[10] = p;
		sb[11] = p >> 8;
		CR (knc_cntrl_transmit (c, sb, 12, rb, &rbs));
	} else {
		sb[6] = n;
		sb[7] = n >> 8;
		sb[8] = p;
		sb[9] = p >> 8;
		CR (knc_cntrl_transmit (c, sb, 10, rb, &rbs));
	}
	CCR (r,rb,rbs);
	CS (rbs, 4);
	return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_get_image (KncCntrl *c, KncCamRes *r, unsigned long n, KncSource s,
	       KncImage t)
{
	unsigned char sb[] = {t, 0x88, 0, 0, s, s >> 8, 0, 0, 0, 0};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);
	KncCntrlProt prot = knc_cntrl_prot (c);

	if (!(prot & KNC_CNTRL_PROT_LONG_ID)) {
                sb[6] = n;
                sb[7] = n >> 8;
		CR (knc_cntrl_transmit (c, sb, 8, rb, &rbs));
        } else {
                sb[6] = n >> 16;
                sb[7] = n >> 24;
                sb[8] = n;
                sb[9] = n >> 8;
		CR (knc_cntrl_transmit (c, sb, 10, rb, &rbs));
        }
	CCR (r,rb,rbs);
	CS (rbs, 4);
        return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_get_image_info (KncCntrl *c, KncCamRes *r, unsigned long n, KncImageInfo *i)
{
	char sb[] = {0x20, 0x88, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);
	KncCntrlProt prot = knc_cntrl_prot (c);

	if (!(prot & KNC_CNTRL_PROT_LONG_ID)) {
                sb[6] = n;
                sb[7] = n >> 8;
		CR (knc_cntrl_transmit (c, sb, 8, rb, &rbs));
		CCR (r,rb,rbs);
		CS (rbs, 10);
		if (i) {
			i->id = (unsigned long) ((rb[5] << 8) | rb[4]);
			i->size = (rb[7] << 8) | rb[6];
			i->prot = (rb[9] << 8) | rb[8];
		}
        } else {
                sb[6] = n >> 16;
                sb[7] = n >> 24;
                sb[8] = n;
                sb[9] = n >> 8;
		CR (knc_cntrl_transmit (c, sb, 10, rb, &rbs));
		CCR (r,rb,rbs);
		CS (rbs, 12);
		if (i) {
			i->id = (rb[5] << 24) | (rb[4] << 16) |
				    (rb[7] << 8 ) |  rb[6];
			i->size = (rb[ 9] << 8) | rb[8];
			i->prot = (rb[11] << 8) | rb[10];
		}
        }
        return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_get_preview (KncCntrl *c, KncCamRes *r, KncPreview p)
{
        unsigned char sb[] = {0x40, 0x88, 0x00, 0x00, p, p >> 8};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 4);
        return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_get_io_pref (KncCntrl *c, KncCamRes *r, KncBitRate *br, KncBitFlag *bf)
{
        unsigned char sb[] = {0x00, 0x90, 0x00, 0x00};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rbs);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 8);
	if (br) *br = (rb[5] << 8) | rb[4];
	if (bf) *br = (rb[7] << 8) | rb[6];
        return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_get_info (KncCntrl *c, KncCamRes *r, KncInfo *i)
{
	const unsigned char sb[] = {0x10, 0x90, 0x00, 0x00};
	unsigned char rb[1024];
	unsigned int rbs = sizeof (rb);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 80);
	if (i) {
		strncpy (i->model,         &rb[ 8],  4);
		strncpy (i->serial_number, &rb[12], 10);
		i->hardware.major =         rb[22];
		i->hardware.minor =         rb[23];
		i->software.major =         rb[24];
		i->software.minor =         rb[25];
		i->testing.major  =         rb[26];
		i->testing.minor  =         rb[27];
		strncpy (i->name,          &rb[28], 22);
		strncpy (i->manufacturer,  &rb[50], 30);
	}
	return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_get_status (KncCntrl *c, KncCamRes *r, KncStatus *s)
{
        unsigned char sb[] = {0x20, 0x90, 0x00, 0x00, 0x00, 0x00};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 34);
	if (s) {
		s->self_test_result = (rb[5] << 8) | rb[4];
		s->power_level      = rb[6];
		s->power_source     = rb[7];
		s->card_status      = rb[8];
		s->display          = rb[9];
		s->card_size        = (rb[11] << 8) | rb[10];
		s->pictures         = (rb[13] << 8) | rb[12];
		s->pictures_left    = (rb[15] << 8) | rb[14];
		s->date.year        = rb[16];
		s->date.month       = rb[17];
		s->date.day         = rb[18];
		s->date.hour        = rb[19];
		s->date.minute      = rb[20];
		s->date.second      = rb[21];
		s->bit_rate         = (rb[23] << 8) | rb[22];
		s->bit_flags        = (rb[25] << 8) | rb[24];
		s->flash            = rb[26];
		s->resolution       = rb[27];
		s->focus            = rb[28];
		s->exposure         = rb[29];
		s->total_pictures   = (rb[31] << 8) | rb[30];
		s->total_strobes    = (rb[33] << 8) | rb[32];
	}
	return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_get_date_and_time (KncCntrl *c, KncCamRes *r, KncDate *d)
{
        const unsigned char sb[] = {0x30, 0x90, 0x00, 0x00};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 10);
	if (d) {
		d->year = rb[4];
		d->month  = rb[5];
		d->day    = rb[6];
		d->hour   = rb[7];
		d->minute = rb[8];
		d->second = rb[9];
	}
	return KNC_CNTRL_RES_OK;
};

KncCntrlRes
knc_get_prefs (KncCntrl *c, KncCamRes *r, KncPrefs *p)
{
	const unsigned char sb[] = {0x40, 0x90, 0x00, 0x00};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 10);
	if (p) {
		p->shutoff_time           = rb[4];
		p->self_timer_time        = rb[5];
		p->beep                   = rb[6];
		p->slide_show_interval    = rb[7];
	}
	return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_set_io_pref (KncCntrl *c, KncCamRes *r, KncBitRate *br, KncBitFlag *bf)
{
        unsigned char sb[] = {0x80, 0x90, 0, 0,
		br ? *br : 0, br ? *br >> 8 : 0,
		bf ? *bf : 0, bf ? *bf >> 8 : 0};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 8);
	if (br) *br = (rb[5] << 8) | rb[4];
	if (bf) *br = (rb[7] << 8) | rb[6];
        return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_set_date_and_time (KncCntrl *c, KncCamRes *r, KncDate d)
{
	const unsigned char sb[] = {0xb0, 0x90, 0, 0, d.year, d.month, d.day,
				    d.hour, d.minute, d.second};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 4);
	return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_set_pref (KncCntrl *c, KncCamRes *r, KncPref p, unsigned int v)
{
	const unsigned char sb[] = {0xc0, 0x90, 0, 0, p, p >> 8, v, v >> 8};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 4);
	return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_reset_prefs (KncCntrl *c, KncCamRes *r)
{
        const unsigned char sb[] = {0xc1, 0x90, 0x00, 0x00};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs)); 
	CCR (r,rb,rbs);
	CS (rbs, 4); 
	return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_take_picture (KncCntrl *c, KncCamRes *r, KncSource s, KncImageInfo *i)
{
	const unsigned char sb[] = {0x00, 0x91, 0x00, 0x00, s, s >> 8};
        unsigned char rb[1024];;
        unsigned int rbs;
	KncCntrlProt prot = knc_cntrl_prot (c);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	if (i) {
		if (prot & KNC_CNTRL_PROT_LONG_ID) {
			CS (rbs, 12);
			i->id = (rb[5] << 24) | (rb[4] << 16) |
				(rb[7] << 8 ) |  rb[6];
			i->size = (rb[ 9] << 8) | rb[8];
			i->prot = (rb[11] << 8) | rb[10];
		} else {
			CS (rbs, 10);
			i->id = (unsigned long) ((rb[5] << 8) | rb[4]);
			i->size = (rb[7] << 8) | rb[6];
			i->prot = (rb[9] << 8) | rb[8];
		}
	}
        return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_loc_tv_output_format_set (KncCntrl *c, KncCamRes *r, KncTVOutputFormat f)
{
	const unsigned char sb[] = {0, 0x92, 0, 0, 0x01, 0, f, f >> 8};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 4);
	return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_loc_date_format_set (KncCntrl *c, KncCamRes *r, KncDateFormat f)
{
	const unsigned char sb[] = {0, 0x92, 0, 0, 0x02, 0, f, f >> 8};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 4);
	return KNC_CNTRL_RES_OK;
}

#define PACKET_SIZE 1024

KncCntrlRes
knc_loc_data_put (KncCntrl *c, KncCamRes *r, const unsigned char *d,
		  unsigned long ds)
{
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);
        unsigned long i = 0, j;
        unsigned char sb[16 + PACKET_SIZE];

	if (ds < 512) return KNC_CNTRL_RES_ERR_ILLEGAL_PARAMETER;

        sb[0] = 0x00;
        sb[1] = 0x92;
        sb[2] = 0x00; /* reserved */
        sb[3] = 0x00; /* reserved */
        sb[4] = 0x00;
        sb[5] = 0x00;
        sb[6] = 0x00;
        sb[7] = 0x00;
        sb[8] = (unsigned char) (PACKET_SIZE >> 0);
        sb[9] = (unsigned char) (PACKET_SIZE >> 8);
        sb[10] = 0x00;
        sb[11] = 0x00;
        sb[12] = 0x00;
        sb[13] = 0x00;
        sb[14] = 0x00;
        sb[15] = 0x00;
        while (1) {

                /* Set up the packet. */
                sb[10] = i >> 16;
                sb[11] = i >> 24;
                sb[12] = i;
                sb[13] = i >> 8;
                for (j = 0; j < PACKET_SIZE; j++) {
                        if ((i + j) < ds) sb[16 + j] = d[i + j];
                        else sb[16 + j] = 0xff;
                }

		/*
                 * We could wait until the camera sends us
		 * K_ERROR_LOCALIZATION_DATA_EXCESS, but it does that
		 * not until the 70000th byte or so. However, as we can
		 * provoke the message with sb[14] = 0x01, we do so as
		 * soon as we exceed the 65535th byte to shorten the
		 * transmission time. We can't do that before or the
		 * camera reports K_ERROR_LOCALIZATION_DATA_CORRUPT.
		 */
		if (i + PACKET_SIZE > 65536)
			sb[14] = 0x01;
		CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
		CS (rbs, 4);
		if ((rb[3] == 0x0b) && (rb[2] == 0x00)) {

			/*
			 * The camera does no longer want to receive
			 * localization data. We are done.
			 */
			return KNC_CNTRL_RES_OK;

		} else CCR (r,rb,rbs);

		/*
		 * Everything is fine. Continue sending
		 * packets but make sure we don't loop
		 * forever.
		 */
		if (i > 131072) return KNC_CNTRL_RES_ERR;

                i += PACKET_SIZE;
        }
}

KncCntrlRes
knc_cancel (KncCntrl *c, KncCamRes *r, KncCmd *cmd)
{
        unsigned char sb[] = {0x00, 0x9e, 0x00, 0x00};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 6);
	if (cmd) *cmd = (rb[5] << 8) | rb[4];
	return KNC_CNTRL_RES_OK;
}
