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

#if 0

KncCntrlRes
knc_erase_image (KncCntrl *p, int image_id_long, unsigned long image_id)
{
        /************************************************/
        /* Command to erase one image.                  */
        /*                                              */
        /* 0x00: Byte 0 of command identifier           */
        /* 0x80: Byte 1 of command identifier           */
        /* 0x00: Reserved                               */
        /* 0x00: Reserved                               */
        /* 0xXX: Byte 0 of p ID                    */
        /*              0x02: Flash memory card         */
        /* 0xXX: Byte 1 of p ID                    */
        /*              0x00: Flash memory card         */
        /* 0xXX: Byte 3 of image ID (QM200 only)        */
        /* 0xXX: Byte 4 of image ID (QM200 only)        */
        /* 0xXX: Byte 0 of image ID                     */
        /* 0xXX: Byte 1 of image ID                     */
        /*                                              */
        /* Return values:                               */
        /* 0x00: Byte 0 of command identifier           */
        /* 0x80: Byte 1 of command identifier           */
        /* 0xXX: Byte 0 of return status                */
        /* 0xXX: Byte 1 of return status                */
        /************************************************/
        unsigned char sb[] = {0x00, 0x80, 0x00, 0x00, 0x02,
                       0x00, 0x00, 0x00, 0x00, 0x00};
        unsigned char *rb = NULL;
        unsigned int rbs;
	KncCntrlRes r;

        if (!image_id_long) {
                sb[6] = image_id;
                sb[7] = image_id >> 8;
		CRF (l_send_receive (p, sb, 8, &rb, &rbs,
							0, NULL, NULL), rb);
        } else {
                sb[6] = image_id >> 16;
                sb[7] = image_id >> 24;
                sb[8] = image_id;
                sb[9] = image_id >> 8;
		CRF (l_send_receive (p, sb, 10, &rb, &rbs, 0, NULL, NULL), rb);
        }

        free (rb);
	r.t = KNC_RES_TYPE_NONE;
        return r;
}


KncCntrlRes
knc_format_memory_card (KncCntrl *p)
{
        /************************************************/
        /* Command to format the memory card.           */
        /*                                              */
        /* 0x10: Byte 0 of command identifier           */
        /* 0x80: Byte 1 of command identifier           */
        /* 0x00: Reserved                               */
        /* 0x00: Reserved                               */
        /* 0xXX: Byte 0 of p ID                    */
        /*              0x02: Flash memory card         */
        /* 0xXX: Byte 1 of p ID                    */
        /*              0x00: Flash memory card         */
        /*                                              */
        /* Return values:                               */
        /* 0x10: Byte 0 of command identifier           */
        /* 0x80: Byte 1 of command identifier           */
        /* 0xXX: Byte 0 of return status                */
        /* 0xXX: Byte 1 of return status                */
        /************************************************/
        unsigned char sb[] = {0x10, 0x80, 0x00, 0x00, 0x02, 0x00};
        unsigned char *rb = NULL;
        unsigned int rbs;
	KncCntrlRes r;

	CRF (l_send_receive (p, sb, 6, &rb, &rbs, 0, NULL, NULL), rb);

        free (rb);
	r.t = KNC_RES_TYPE_NONE;
        return r;
}


KncCntrlRes
knc_erase_all (KncCntrl *p, 
	     unsigned int *number_of_images_not_erased)
{
        /************************************************/
        /* Command to erase all images in the camera,   */
        /* except the protected ones.                   */
        /*                                              */
        /* 0x20: Byte 0 of command identifier           */
        /* 0x80: Byte 1 of command identifier           */
        /* 0x00: Reserved                               */
        /* 0x00: Reserved                               */
        /* 0xXX: Byte 0 of p ID                    */
        /*              0x02: Flash memory card         */
        /* 0xXX: Byte 1 of p ID                    */
        /*              0x00: Flash memory card         */
        /*                                              */
        /* Return values:                               */
        /* 0x20: Byte 0 of command identifier           */
        /* 0x80: Byte 1 of command identifier           */
        /* 0xXX: Byte 0 of return status                */
        /* 0xXX: Byte 1 of return status                */
        /* Following bytes only in case of success.     */
        /* 0xXX: Byte 0 of number of images not erased  */
        /* 0xXX: Byte 1 of number of images not erased  */
        /************************************************/
        unsigned char sb[] = {0x20, 0x80, 0x00, 0x00, 0x02, 0x00};
        unsigned char *rb = NULL;
        unsigned int rbs;
	KncCntrlRes r;

	CHECK_NULL (number_of_images_not_erased);

	CRF (l_send_receive (p, sb, 6, &rb, &rbs, 0, NULL, NULL), rb);

	*number_of_images_not_erased = (rb[5] << 8) | rb[4];
        free (rb);
	r.t = KNC_RES_TYPE_NONE;
        return r;
}


KncCntrlRes
knc_set_protect_status (KncCntrl *p,  int image_id_long,
		      unsigned long image_id, int protected)
{
        /************************************************/
        /* Command to set the protect status of one     */
        /* image.                                       */
        /*                                              */
        /* 0x30: Byte 0 of command identifier           */
        /* 0x80: Byte 1 of command identifier           */
        /* 0x00: Reserved                               */
        /* 0x00: Reserved                               */
        /* 0xXX: Byte 0 of p ID                    */
        /*              0x02: Flash memory card         */
        /* 0xXX: Byte 1 of p ID                    */
        /*              0x00: Flash memory card         */
        /* 0xXX: Byte 3 of image ID (QM200 only)        */
        /* 0xXX: Byte 4 of image ID (QM200 only)        */
        /* 0xXX: Byte 0 of image ID                     */
        /* 0xXX: Byte 1 of image ID                     */
        /* 0xXX: Byte 0 of protect status               */
        /*              0x00: not protected             */
        /*              0x01: protected                 */
        /* 0x00: Byte 1 of protect status               */
        /*                                              */
        /* Return values:                               */
        /* 0x30: Byte 0 of command identifier           */
        /* 0x80: Byte 1 of command identifier           */
        /* 0xXX: Byte 0 of return status                */
        /* 0xXX: Byte 1 of return status                */
        /************************************************/
        unsigned char sb[] = {0x30, 0x80, 0x00, 0x00, 0x02, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        unsigned char *rb = NULL;
        unsigned int rbs;
	KncCntrlRes r;

        if (!image_id_long) {
                if (protected) sb[8] = 0x01;
                sb[6] = image_id;
                sb[7] = image_id >> 8;
		CRF (l_send_receive (p, sb, 10, &rb, &rbs,
						0, NULL, NULL), rb);
        } else {
                if (protected) sb[10] = 0x01;
                sb[6] = image_id >> 16;
                sb[7] = image_id >> 24;
                sb[8] = image_id;
                sb[9] = image_id >> 8;
		CRF (l_send_receive (p, sb, 12, &rb, &rbs,
						0, NULL, NULL), rb);
        }

        free (rb);
	r.t = KNC_RES_TYPE_NONE;
        return r;
}
#endif

KncCntrlRes
knc_get_image (KncCntrl *c, unsigned long n, KncSource s, KncImage t,
	       KncCamRes *r)
{
	unsigned char sb[] = {t, 0x88, 0, 0, s, s >> 8, 0, 0, 0, 0};
        unsigned char rb[1024];
        unsigned int sbs, rbs = sizeof (rb);
	KncCntrlProt prot;

	prot = knc_cntrl_prot (c);
	if (!(prot & KNC_CNTRL_PROT_LONG_ID)) {
                sb[6] = n;
                sb[7] = n >> 8;
		sbs = 8;
        } else {
                sb[6] = n >> 16;
                sb[7] = n >> 24;
                sb[8] = n;
                sb[9] = n >> 8;
		sbs = 10;
        }
	CR (knc_cntrl_transmit (c, sb, sbs, rb, &rbs));
	CCR (r,rb,rbs);
	CS (rbs, 4);

        return KNC_CNTRL_RES_OK;
}

KncCntrlRes
knc_get_image_info (KncCntrl *c, unsigned long n, KncCamRes *r, KncImageInfo *i)
{
	char sb[] = {0x20, 0x88, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0};
        unsigned char rb[1024];
        unsigned int rbs = sizeof (rb);
	KncCntrlProt prot;

	prot = knc_cntrl_prot (c);

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

#if 0

KncCntrlRes
knc_get_status (KncCntrl *p,  KStatus *status)
{
        /************************************************/
        /* Command to get the status of the camera.     */
        /*                                              */
        /* 0x20: Byte 0 of command identifier           */
        /* 0x90: Byte 1 of command identifier           */
        /* 0x00: Reserved                               */
        /* 0x00: Reserved                               */
        /* (...)                                        */
        /*                                              */
        /* You can add pairs of additional bytes. If    */
        /* those are all 0x00, then nothing will        */
        /* change. If at least one deviates, all        */
        /* individual pieces of the status information  */
        /* will be returned as being zero.              */
        /*                                              */
        /* Return values:                               */
        /* 0x20: Byte 0 of command identifier           */
        /* 0x90: Byte 1 of command identifier           */
        /* 0xXX: Byte 0 of return status                */
        /* 0xXX: Byte 1 of return status                */
        /* Following bytes only in case of success.     */
        /* 0xXX: Result of self test                    */
        /*              0x00: Self test passed          */
        /*              other: Self test failed         */
        /* 0xXX: Power level                            */
        /*              0x00: Low                       */
        /*              0x01: Normal                    */
        /*              0x02: High                      */
        /* 0xXX: Power source                           */
        /*              0x00: Battery                   */
        /*              0x01: AC                        */
        /* 0xXX: Card status                            */
        /*              0x07: Card                      */
        /*              0x12: No card                   */
        /* 0xXX: Display                                */
        /*              0x00: built in                  */
        /*              0x02: TV                        */
        /* 0xXX: Byte 0 of card size                    */
        /* 0xXX: Byte 1 of card size                    */
        /* 0xXX: Byte 0 of pictures in camera           */
        /* 0xXX: Byte 1 of left pictures in camera      */
        /* 0xXX: Year                                   */
        /* 0xXX: Month                                  */
        /* 0xXX: Day                                    */
        /* 0xXX: Hour                                   */
        /* 0xXX: Minute                                 */
        /* 0xXX: Second                                 */
        /* 0xXX: Byte 0 of bit rates                    */
        /* 0xXX: Byte 1 of bit rates                    */
        /* 0xXX: Byte 0 of bit flags                    */
        /* 0xXX: Byte 1 of bit flags                    */
        /* 0xXX: Flash                                  */
        /* 0xXX: Resolution                             */
        /* 0xXX: Focus                                  */
        /* 0xXX: Exposure                               */
        /* 0xXX: Byte 0 of total pictures               */
        /* 0xXX: Byte 1 of total pictures               */
        /* 0xXX: Byte 0 of total strobes                */
        /* 0xXX: Byte 1 of total strobes                */
        /************************************************/
        unsigned char sb[] = {0x20, 0x90, 0x00, 0x00, 0x00, 0x00};
        unsigned char *rb = NULL;
        unsigned int rbs;
	KncCntrlRes r;

	CHECK_NULL (status);

        CRF (l_send_receive (p, sb, 6, &rb, &rbs, 0, NULL, NULL), rb);
	
	status->self_test_result = (rb[5] << 8) | rb[4];
	status->power_level      = rb[6];
	status->power_source     = rb[7];
	status->card_status      = rb[8];
	status->display          = rb[9];
	status->card_size        = (rb[11] << 8) | rb[10];
	status->pictures         = (rb[13] << 8) | rb[12];
	status->pictures_left    = (rb[15] << 8) | rb[14];
	status->date.year        = rb[16];
	status->date.month       = rb[17];
	status->date.day         = rb[18];
	status->date.hour        = rb[19];
	status->date.minute      = rb[20];
	status->date.second      = rb[21];
	status->bit_rate         = (rb[23] << 8) | rb[22];
	status->bit_flags        = (rb[25] << 8) | rb[24];
	status->flash            = rb[26];
	status->resolution       = rb[27];
	status->focus            = rb[28];
	status->exposure         = rb[29];
	status->total_pictures   = (rb[31] << 8) | rb[30];
	status->total_strobes    = (rb[33] << 8) | rb[32];

        free (rb);
	r.t = KNC_RES_TYPE_NONE;
        return r;
}

KncCntrlRes
knc_get_date_and_time (KncCntrl *p,  KDate *date)
{
        /************************************************/
        /* Command to get the date and time from the    */
        /* camera.                                      */
        /*                                              */
        /* 0x30: Byte 0 of command identifier           */
        /* 0x90: Byte 1 of command identifier           */
        /* 0x00: Reserved                               */
        /* 0x00: Reserved                               */
        /*                                              */
        /* Return values:                               */
        /* 0x30: Byte 0 of command identifier           */
        /* 0x90: Byte 1 of command identifier           */
        /* 0xXX: Byte 0 of return status                */
        /* 0xXX: Byte 1 of return status                */
        /* Following bytes only in case of success.     */
        /* 0xXX: Year                                   */
        /* 0xXX: Month                                  */
        /* 0xXX: Day                                    */
        /* 0xXX: Hour                                   */
        /* 0xXX: Minute                                 */
        /* 0xXX: Second                                 */
        /************************************************/
        unsigned char sb[] = {0x30, 0x90, 0x00, 0x00};
        unsigned char *rb = NULL;
        unsigned int rbs;
	KncCntrlRes r;

        CRF (l_send_receive (p, sb, 4, &rb, &rbs, 0, NULL, NULL), rb);
	date->year   = rb[4];
	date->month  = rb[5];
	date->day    = rb[6];
	date->hour   = rb[7];
	date->minute = rb[8];
	date->second = rb[9];

        free (rb);
	r.t = KNC_RES_TYPE_NONE;
        return r;
};

KncCntrlRes
knc_get_preferences (KncCntrl *p, KPreferences *preferences)
{
        /************************************************/
        /* Command to get the preferences from the      */
        /* camera.                                      */
        /*                                              */
        /* 0x40: Byte 0 of command identifier           */
        /* 0x90: Byte 1 of command identifier           */
        /* 0x00: Reserved                               */
        /* 0x00: Reserved                               */
        /*                                              */
        /* Return values:                               */
        /* 0x40: Byte 0 of command identifier           */
        /* 0x90: Byte 1 of command identifier           */
        /* 0xXX: Byte 0 of return status                */
        /* 0xXX: Byte 1 of return status                */
        /* Following bytes only in case of success.     */
        /* 0xXX: Byte 0 of shutoff time                 */
        /* 0xXX: Byte 1 of shutoff time                 */
        /* 0xXX: Byte 0 of self timer time              */
        /* 0xXX: Byte 1 of self timer time              */
        /* 0xXX: Byte 0 of beep                         */
        /* 0xXX: Byte 1 of beep                         */
        /* 0xXX: Byte 0 of slide show interval          */
        /* 0xXX: Byte 1 of slide show interval          */
        /************************************************/
        unsigned char sb[] = {0x40, 0x90, 0x00, 0x00};
        unsigned char *rb = NULL;
        unsigned int rbs;
	KncCntrlRes r;

        CRF (l_send_receive (p, sb, 4, &rb, &rbs, 0, NULL, NULL), rb);
	preferences->shutoff_time           = rb[4];
	preferences->self_timer_time        = rb[5];
	preferences->beep                   = rb[6];
	preferences->slide_show_interval    = rb[7];

        free (rb);
	r.t = KNC_RES_TYPE_NONE;
        return r;
}

#endif

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

#if 0

KncCntrlRes
knc_set_date_and_time (KncCntrl *p, KDate date)
{
        /************************************************/
        /* Command to set date and time of the camera.  */
        /*                                              */
        /* 0xb0: Byte 0 of command identifier           */
        /* 0x90: Byte 1 of command identifier           */
        /* 0x00: Reserved                               */
        /* 0x00: Reserved                               */
        /* 0xXX: Year   (0x00 to 0x25, 0x60 to 0x63)    */
        /* 0xXX: Month  (0x01 to 0x0C)                  */
        /* 0xXX: Day    (0x01 to 0x1F)                  */
        /* 0xXX: Hour   (0x00 to 0x17)                  */
        /* 0xXX: Minute (0x00 to 0x3b)                  */
        /* 0xXX: Second (0x00 to 0x3b)                  */
        /*                                              */
        /* Return values:                               */
        /* 0xb0: Byte 0 of command identifier           */
        /* 0x90: Byte 1 of command identifier           */
        /* 0xXX: Byte 0 of return status                */
        /* 0xXX: Byte 1 of return status                */
        /************************************************/
        unsigned char sb[10];
        unsigned char *rb = NULL;
        unsigned int rbs;
	KncCntrlRes r;

	sb[0] = 0xb0;
	sb[1] = 0x90;
	sb[2] = 0x00; /* reserved */
	sb[3] = 0x00; /* reserved */
        sb[4] = date.year;
        sb[5] = date.month;
        sb[6] = date.day;
        sb[7] = date.hour;
        sb[8] = date.minute;
        sb[9] = date.second;
	CRF (l_send_receive (p, sb, 10, &rb, &rbs, 0, NULL, NULL), rb);
        free (rb);
	r.t = KNC_RES_TYPE_NONE;
        return r;
}


KncCntrlRes
knc_set_preference (KncCntrl *p, KPreference preference, unsigned int value)
{
        /* Return values:                               */
        /* 0xc0: Byte 0 of command identifier           */
        /* 0x90: Byte 1 of command identifier           */
        /* 0xXX: Byte 0 of return status                */
        /* 0xXX: Byte 1 of return status                */
        unsigned char sb[8];
        unsigned char *rb = NULL;
        unsigned int rbs;
	KncCntrlRes r;

	sb[0] = 0xc0;
	sb[1] = 0x90;
	sb[2] = 0x00; /* reserved */
	sb[3] = 0x00; /* reserved */
	sb[4] = preference >> 0;
	sb[5] = preference >> 8;
	sb[6] = value >> 0;
	sb[7] = value >> 8;
	CRF (l_send_receive (p, sb, 8, &rb, &rbs, 0, NULL, NULL), rb);
        free (rb);
	r.t = KNC_RES_TYPE_NONE;
        return r;
}

KncCntrlRes
knc_reset_preferences (KncCntrl *p)
{
        /************************************************/
        /* Command to reset the preferences of the      */
        /* camera.                                      */
        /*                                              */
        /* 0xc1: Byte 0 of command identifier           */
        /* 0x90: Byte 1 of command identifier           */
        /* 0x00: Reserved                               */
        /* 0x00: Reserved                               */
        /*                                              */
        /* Return values:                               */
        /* 0xc1: Byte 0 of command identifier           */
        /* 0x90: Byte 1 of command identifier           */
        /* 0xXX: Byte 0 of return status                */
        /* 0xXX: Byte 1 of return status                */
        /************************************************/
        unsigned char sb[] = {0xc1, 0x90, 0x00, 0x00};
        unsigned char *rb = NULL;
        unsigned int rbs;
	KncCntrlRes r;

	CRF (l_send_receive (p, sb, 4, &rb, &rbs, 0, NULL, NULL), rb);
        free (rb);
	r.t = KNC_RES_TYPE_NONE;
	return r;
}
#endif

KncCntrlRes
knc_take_picture (KncCntrl *c, KncSource s, KncCamRes *r, KncImageInfo *i)
{
	const unsigned char sb[] = {0x00, 0x91, 0x00, 0x00, s, s >> 8};
        unsigned char rb[1024];;
        unsigned int rbs;
	KncCntrlProt prot;

	CR (knc_cntrl_transmit (c, sb, sizeof (sb), rb, &rbs));
	CCR (r,rb,rbs);
	if (i) {
		prot = knc_cntrl_prot (c);
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
knc_loc_tv_output_format_set (KncCntrl *c, KncTVOutputFormat f, KncCamRes *r)
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
knc_loc_date_format_set (KncCntrl *c, KncDateFormat f, KncCamRes *r)
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
knc_loc_data_put (KncCntrl *c, const unsigned char *d, unsigned long ds,
		  KncCamRes *r)
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
