/* knc.h
 *
 * Copyright (C) 2002 Lutz Müller <urc8@rz.uni-karlsruhe.de>
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

#ifndef __KNC_H__
#define __KNC_H__

#include <libknc/knc-cmd.h>
#include <libknc/knc-cntrl.h>
#include <libknc/knc-cam-res.h>

typedef enum {
	KNC_SOURCE_CARD = 0x0002
} KncSource;

typedef struct {
	unsigned char major;
	unsigned char minor;
} KncVersion;

typedef struct {
	char model[5];
	char serial_number[11];
	KncVersion hardware;
	KncVersion software;
	KncVersion testing;
	char name[23];
	char manufacturer[31];
} KncInfo;

KncCntrlRes knc_get_info (KncCntrl *, KncCamRes *, KncInfo *);

typedef enum {
	KNC_PROT_NO  = 0x00,
	KNC_PROT_YES = 0x01
} KncProt;

typedef struct {
	unsigned long id;
	unsigned int size;
	KncProt prot;
} KncImageInfo;

KncCntrlRes knc_get_image_info (KncCntrl *, unsigned long, KncCamRes *,
				KncImageInfo *);

#if 0
/*
 * The following are the preferences that can be set. The
 * sixteen bits that identify the preference are as follows.
 * Low order byte:	0000	0aa0
 * High order byte:	1x0b	xxxx
 * x can be 1 or 0 with no effect on the command. The a and b
 * bits identify the preference and are as follows.
 *
 *	b:	0			1
 * aa:	----------------------------------------------------
 * 00	|	resolution		flash
 * 01	|	exposure		focus / self timer
 * 10	|	self timer time		auto off time
 * 11	|	slide show interval	beep
 *
 * A short explanation of the values of the preferences:
 *
 * RESOLUTION (default: 2)
 * Only the low order byte is processed.
 *  - 0: medium resolution (same as 2)
 *  - 1: high resolution (1152 x 872)
 *  - 2: medium resolution (1152 x 872)
 *  - 3: low resolution	( 576 x 436)
 * Other values are accepted as well, produce however strange
 * results on the display, as the camera will calculate the
 * remaining pictures on the basis of this value. Those values
 * are at some point changed to 'normal' ones.
 * Each pixel has an area of 4.6um x 4.6 um.
 *
 * EXPOSURE (default: 0)
 * Only the low order byte is processed.
 *  - 0 to 255.
 * The behaviour is somewhat strange. Sending a value will
 * not necessarily set the value to exactly the sent one.
 * The following table gives an overview.
 * 	sent	received	sent	received
 *	----------------	----------------
 *	  0  -	  0		128	129
 *	  1	  0		129	130
 *	  2	  1		131  -	131
 *	  3	  2		132	133
 *	  4	  3		133	134
 *	  5  -	  5		134	135
 *	  6	  5		135	136
 *	  7	  6		136  -	136
 *	  8	  7		   (...)
 *	  9	  8		246  -	246
 *	 10  -	 10		247	248
 *	   (...)		248	249
 *	120  -	120		249	250
 *	121	120		250	251
 *	122	121		251  -	251
 *	123	122		252	253
 *	124	123		253	254
 *	125  -	125		254	255
 *	126	125		255	  0
 *	127	126
 *
 * Additional information from HP:
 *  - Range: EV 6 to EV 16 (ISO 100).
 *  - Exposure times: 1/500th to 2 (sec)
 *
 * SELF_TIMER_TIME (default: 10)
 *  - 3 to 40. Measured in seconds.
 * All other values are rejected.
 *
 * SLIDE_SHOW_INTERVAL (default: 3)
 *  - 1 to 30. Measured in seconds.
 * All other values are rejected.
 *
 * FLASH (default: 2)
 * Only the low order byte is processed.
 *  - 0: off
 *  - 1: on
 *  - 2: auto
 *  - 5: on, redeye
 *  - 6: auto, redeye
 * Other values are accepted as well, but will be changed to 2
 * at some point.
 *
 * FOCUS_SELF_TIMER (default: 0)
 * Only the low order byte is processed.
 *  - 0: fixed, next picture in normal mode
 *  - 1: fixed, next picture in self timer mode
 *  - 2: auto, next picture in normal mode
 *  - 3: auto, next picture in self timer mode
 * After the next picture has been taken in self timer mode,
 * the value is automatically changed to the next lower even
 * value (normal mode).
 *
 * Additional information from HP:
 *  - Fixed: 2.6 feet to infinity
 *  - Auto: 8 inches to infinity
 *
 * AUTO_OFF_TIME (default: 5)
 * Only the low order byte is processed.
 *  - 0: Will be changed by the camera to 255.
 *  - 1 to 255. Measured in minutes.
 *
 * BEEP (default: 1)
 * Only the low order byte is processed.
 *  - 0: off
 *  - 1 to 255: on
 */
typedef enum {
	KNC_PREFERENCE_RESOLUTION          = 0xc000,
	KNC_PREFERENCE_EXPOSURE            = 0xc002,
	KNC_PREFERENCE_SELF_TIMER_TIME     = 0xc004,
	KNC_PREFERENCE_SLIDE_SHOW_INTERVAL = 0xc006,
	KNC_PREFERENCE_FLASH               = 0xd000,
	KNC_PREFERENCE_FOCUS_SELF_TIMER    = 0xd002,
	KNC_PREFERENCE_AUTO_OFF_TIME       = 0xd004,
	KNC_PREFERENCE_BEEP                = 0xd006
} KPreference;

typedef enum {
	KNC_POWER_LEVEL_LOW    = 0x00, 
	KNC_POWER_LEVEL_NORMAL = 0x01,
	KNC_POWER_LEVEL_HIGH   = 0x02
} KPowerLevel;

typedef enum {
	KNC_POWER_SOURCE_BATTERY = 0x00,
	KNC_POWER_SOURCE_AC      = 0x01
} KPowerSource;

typedef enum {
	KNC_DISPLAY_BUILT_IN = 0x00,
	KNC_DISPLAY_TV       = 0x02
} KDisplay;

typedef enum {
	KNC_CARD_STATUS_CARD    = 0x07,
	KNC_CARD_STATUS_NO_CARD = 0x12
} KCardStatus;

#endif

/* Managing the connection */

typedef enum {
	KNC_BIT_RATE_300    = 1 << 0,
	KNC_BIT_RATE_600    = 1 << 1,
	KNC_BIT_RATE_1200   = 1 << 2,
	KNC_BIT_RATE_2400   = 1 << 3,
	KNC_BIT_RATE_4800   = 1 << 4,
	KNC_BIT_RATE_9600   = 1 << 5,
	KNC_BIT_RATE_19200  = 1 << 6,
	KNC_BIT_RATE_38400  = 1 << 7,
	KNC_BIT_RATE_57600  = 1 << 8,
	KNC_BIT_RATE_115200 = 1 << 9
} KncBitRate;

typedef enum {
	KNC_BIT_FLAG_8_BITS       = 1 << 0,
	KNC_BIT_FLAG_STOP_2_BITS  = 1 << 1,
	KNC_BIT_FLAG_PARITY_ON    = 1 << 2,
	KNC_BIT_FLAG_PARITY_ODD   = 1 << 3,
	KNC_BIT_FLAG_HW_FLOW_CTRL = 1 << 4
} KncBitFlag;

KncCntrlRes knc_get_io_pref (KncCntrl *, KncCamRes *,
			     KncBitRate *, KncBitFlag *);
KncCntrlRes knc_set_io_pref (KncCntrl *, KncCamRes *,
			     KncBitRate *, KncBitFlag *);

#if 0

KncCntrlRes knc_erase_all (KncCntrl *, unsigned int *number_of_images_not_erased);

KncCntrlRes knc_format_memory_card (KncCntrl *);

#endif

/* Capturing images and previews */

typedef enum {
	KNC_PREVIEW_YES = 0x0001,
	KNC_PREVIEW_NO  = 0x0000
} KncPreview;

KncCntrlRes knc_get_preview  (KncCntrl *, KncCamRes *, KncPreview);
KncCntrlRes knc_take_picture (KncCntrl *, KncSource, KncCamRes *,
			      KncImageInfo *);

#if 0
KncCntrlRes knc_set_protect_status (KncCntrl *, int image_id_long,
			  unsigned long image_id, int protected);


KncCntrlRes knc_erase_image (KncCntrl *, int image_id_long, unsigned long image_id);


KncCntrlRes knc_reset_preferences (KncCntrl *);

typedef struct {
	unsigned char year, month, day;
	unsigned char hour, minute, second;
} KDate;

KncCntrlRes knc_get_date_and_time (KncCntrl *, KDate *);
KncCntrlRes knc_set_date_and_time (KncCntrl *, KDate  );

typedef struct {
	unsigned int shutoff_time;
	unsigned int self_timer_time;
	unsigned int beep;
	unsigned int slide_show_interval;
} KPreferences;

KncCntrlRes knc_get_preferences (KncCntrl *, KPreferences *);
KncCntrlRes knc_set_preference  (KncCntrl *, KPreference, unsigned int value);

typedef struct {
	KPowerLevel  power_level;
	KPowerSource power_source;
	KCardStatus  card_status;
	KDisplay     display;
	unsigned int self_test_result;
	unsigned int card_size;
	unsigned int pictures, pictures_left;
	KDate date;
	unsigned int bit_rate;
	KBitFlag bit_flags;
	unsigned char flash, resolution, focus, exposure;
	unsigned char total_pictures, total_strobes;
} KStatus;

KncCntrlRes knc_get_status (KncCntrl *, KStatus *);
#endif

/* Getting images */

typedef enum {
	KNC_IMAGE_THUMB = 0x00,
	KNC_IMAGE_JPEG  = 0x10,
	KNC_IMAGE_EXIF  = 0x30
} KncImage;

KncCntrlRes knc_get_image (KncCntrl *, unsigned long, KncSource, KncImage,
			   KncCamRes *);

/* Localization */

typedef enum {
	KNC_TV_OUTPUT_FORMAT_NTSC = 0x0000,
	KNC_TV_OUTPUT_FORMAT_PAL  = 0x0001,
	KNC_TV_OUTPUT_FORMAT_HIDE = 0x0002
} KncTVOutputFormat;
KncCntrlRes knc_loc_tv_output_format_set (KncCntrl *, KncTVOutputFormat,
					  KncCamRes *);

typedef enum {
	KNC_DATE_FORMAT_MONTH_DAY_YEAR = 0x0000,
	KNC_DATE_FORMAT_DAY_MONTH_YEAR = 0x0001,
	KNC_DATE_FORMAT_YEAR_MONTH_DAY = 0x0002
} KncDateFormat;
KncCntrlRes knc_loc_date_format_set (KncCntrl *, KncDateFormat, KncCamRes *);

KncCntrlRes knc_loc_data_put (KncCntrl *, const unsigned char *, unsigned long,
			      KncCamRes *);

/* Cancelling commands. Don't use this function. */

KncCntrlRes knc_cancel (KncCntrl *, KncCamRes *, KncCmd *);

#endif /* __KNC_H__ */
