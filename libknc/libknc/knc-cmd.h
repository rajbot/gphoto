#ifndef __KONICA_COMMAND_H__
#define __KONICA_COMMAND_H__

/*
 * The following are the commands that are supported by the 
 * camera. All other commands will be rejected as unsupported.
 *
 * 0x9200:      On 'HP PhotoSmart C20', this command seems to
 *              set the language by transferring huge amounts
 *              of data. For 'Q-M100', this command is not
 *              defined according to Knc.
 * 0x9e10:      Used for development and testing of the camera.
 * 0x9e20:      Used for development and testing of the camera.
 *
 * For those commands, no documentation is available. They are
 * therefore not implemented.
 */
typedef enum {
	KNC_ERASE_IMAGE                 = 0x8000,
        KNC_FORMAT_MEMORY_CARD          = 0x8010,
        KNC_ERASE_ALL                   = 0x8020,
        KNC_SET_PROT          		= 0x8030,
        KNC_GET_IMAGE_THUMB             = 0x8800,
        KNC_GET_IMAGE_JPEG              = 0x8810,
        KNC_GET_IMAGE_INFO		= 0x8820,
        KNC_GET_IMAGE_EXIF              = 0x8830,
        KNC_GET_PREVIEW                 = 0x8840,
        KNC_GET_IO_PREF			= 0x9000,
        KNC_GET_INFO			= 0x9010,
        KNC_GET_STATUS                  = 0x9020,
        KNC_GET_DATE_AND_TIME           = 0x9030,
        KNC_GET_PREFS             	= 0x9040,
        KNC_SET_IO_PREF			= 0x9080,
        KNC_SET_DATE_AND_TIME           = 0x90b0,
        KNC_SET_PREF              	= 0x90c0,
        KNC_RESET_PREFS           	= 0x90c1,
        KNC_TAKE_PICTURE                = 0x9100,
        KNC_LOCALIZATION		= 0x9200,
        KNC_CANCEL                      = 0x9e00
        /* ?                            = 0x9e10, */
        /* ?                            = 0x9e20, */
} KncCmd;

const char  *knc_cmd_name (KncCmd);
unsigned int knc_cmd_data (KncCmd);

#endif
