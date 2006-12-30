/* knc.h
 *
 * Copyright (C) 2002 Lutz Müller <lutz@users.sf.net>
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

typedef enum {
	KNC_POWER_LEVEL_LOW    = 0x00, 
	KNC_POWER_LEVEL_NORMAL = 0x01,
	KNC_POWER_LEVEL_HIGH   = 0x02
} KncPowerLevel;

typedef enum {
	KNC_POWER_SOURCE_BATTERY = 0x00,
	KNC_POWER_SOURCE_AC      = 0x01
} KncPowerSource;

typedef enum {
	KNC_DISPLAY_BUILT_IN = 0x00,
	KNC_DISPLAY_TV       = 0x02
} KncDisplay;

typedef enum {
	KNC_CARD_STATUS_CARD    = 0x07,
	KNC_CARD_STATUS_NO_CARD = 0x12
} KncCardStatus;

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

/* Erasing multiple images simultaneously */

KncCntrlRes knc_erase_all (KncCntrl *, KncCamRes *, KncSource, unsigned int *);
KncCntrlRes knc_format    (KncCntrl *, KncCamRes *, KncSource);

/* Capturing images and previews */

typedef enum {
	KNC_PREVIEW_YES = 0x0001,
	KNC_PREVIEW_NO  = 0x0000
} KncPreview;

KncCntrlRes knc_get_preview  (KncCntrl *, KncCamRes *, KncPreview);
KncCntrlRes knc_take_picture (KncCntrl *, KncCamRes *, KncSource,
			      KncImageInfo *);

/* Accessing date and time */

typedef struct {
	unsigned char year, month, day;
	unsigned char hour, minute, second;
} KncDate;

KncCntrlRes knc_get_date_and_time (KncCntrl *, KncCamRes *, KncDate *);
KncCntrlRes knc_set_date_and_time (KncCntrl *, KncCamRes *, KncDate);

/* Accessing preferences */

typedef enum {
	KNC_PREF_RESOLUTION          = 0xc000,
	KNC_PREF_EXPOSURE            = 0xc002,
	KNC_PREF_SELF_TIMER_TIME     = 0xc004,
	KNC_PREF_SLIDE_SHOW_INTERVAL = 0xc006,
	KNC_PREF_FLASH               = 0xd000,
	KNC_PREF_FOCUS_SELF_TIMER    = 0xd002,
	KNC_PREF_AUTO_OFF_TIME       = 0xd004,
	KNC_PREF_BEEP                = 0xd006
} KncPref;

typedef struct {
	unsigned int auto_off_time;
	unsigned int self_timer_time;
	unsigned int beep;
	unsigned int slide_show_interval;
} KncPrefs;

KncCntrlRes knc_get_prefs   (KncCntrl *, KncCamRes *, KncPrefs *);
KncCntrlRes knc_set_pref    (KncCntrl *, KncCamRes *, KncPref, unsigned int);
KncCntrlRes knc_reset_prefs (KncCntrl *, KncCamRes *);

/* Status information */

typedef enum {
	KNC_FLASH_OFF		= 0x00,
	KNC_FLASH_ON  		= 0x01,
	KNC_FLASH_AUTO		= 0x02,
	KNC_FLASH_ON_RED_EYE	= 0x05,
	KNC_FLASH_AUTO_RED_EYE	= 0x06
} KncFlash;

typedef enum {
	KNC_FOCUS_SELF_TIMER_FIXED		= 0x00,
	KNC_FOCUS_SELF_TIMER_FIXED_SELF_TIMER	= 0x01,
	KNC_FOCUS_SELF_TIMER_AUTO		= 0x02,
	KNC_FOCUS_SELF_TIMER_AUTO_SELF_TIMER	= 0x03
} KncFocusSelfTimer;

typedef enum {
	KNC_RESOLUTION_HIGH	= 0x01,
	KNC_RESOLUTION_MEDIUM	= 0x02,
	KNC_RESOLUTION_LOW	= 0x03
} KncResolution;

typedef struct {
	KncPowerLevel  power_level;
	KncPowerSource power_source;
	KncCardStatus  card_status;
	KncDisplay     display;
	unsigned int self_test_result;
	unsigned int card_size;
	unsigned int pictures, pictures_left;
	KncDate date;
	KncBitRate bit_rate;
	KncBitFlag bit_flags;
	KncFlash flash;
	KncFocusSelfTimer focus_self_timer;
	KncResolution resolution;
	unsigned char exposure;
	unsigned char total_pictures, total_strobes;
} KncStatus;

KncCntrlRes knc_get_status (KncCntrl *, KncCamRes *, KncStatus *);

/* Accessing images */

typedef enum {
	KNC_IMAGE_THUMB = 0x00,
	KNC_IMAGE_JPEG  = 0x10,
	KNC_IMAGE_EXIF  = 0x30
} KncImage;

KncCntrlRes knc_get_image      (KncCntrl *, KncCamRes *, unsigned long,
				KncSource, KncImage);
KncCntrlRes knc_set_prot       (KncCntrl *, KncCamRes *, unsigned long,
				KncSource, KncProt);
KncCntrlRes knc_erase_image    (KncCntrl *, KncCamRes *, unsigned long,
				KncSource);
KncCntrlRes knc_get_image_info (KncCntrl *, KncCamRes *, unsigned long,
				KncImageInfo *);

/* Localization */

typedef enum {
	KNC_TV_OUTPUT_FORMAT_NTSC = 0x0000,
	KNC_TV_OUTPUT_FORMAT_PAL  = 0x0001,
	KNC_TV_OUTPUT_FORMAT_HIDE = 0x0002
} KncTVOutputFormat;
KncCntrlRes knc_loc_tv_output_format_set (KncCntrl *, KncCamRes *,
					  KncTVOutputFormat);

typedef enum {
	KNC_DATE_FORMAT_MDY = 0x0000,
	KNC_DATE_FORMAT_DMY = 0x0001,
	KNC_DATE_FORMAT_YMD = 0x0002
} KncDateFormat;
KncCntrlRes knc_loc_date_format_set (KncCntrl *, KncCamRes *, KncDateFormat);

KncCntrlRes knc_loc_data_put        (KncCntrl *, KncCamRes *,
				     const unsigned char *, unsigned long);

/* Cancelling commands. Don't use this function. */

KncCntrlRes knc_cancel (KncCntrl *, KncCamRes *, KncCmd *);

#endif /* __KNC_H__ */
