#include <config.h>
#include "knc-cam-res.h"
#include "knc-i18n.h"

#include <stdlib.h>

static struct {
	KncCamRes r;
	const char *name;
} KncCamResNames[] = {
	{KNC_CAM_RES_OK, N_("No error")},
	{KNC_CAM_RES_ERR_FOCUS, N_("Focusing error")},
	{KNC_CAM_RES_ERR_IRIS, N_("Iris error")},
	{KNC_CAM_RES_ERR_STROBE, N_("Focusing error")},
	{KNC_CAM_RES_ERR_EEPROM, N_("EEPROM checksum error")},
	{KNC_CAM_RES_ERR_INTERNAL_1, N_("Internal error (1)")},
	{KNC_CAM_RES_ERR_INTERNAL_2, N_("Internal error (2)")},
	{KNC_CAM_RES_ERR_NO_CARD, N_("No card present")},
	{KNC_CAM_RES_ERR_CARD_NOT_SUPPORTED, N_("Card not supported")},
	{KNC_CAM_RES_ERR_CARD_REMOVED, N_("Card removed during access")},
	{KNC_CAM_RES_ERR_INVALID_IMAGE_NUMBER, N_("Image number not valid")},
	{KNC_CAM_RES_ERR_CARD_CANNOT_BE_WRITTEN, N_("Card cannot be written")},
	{KNC_CAM_RES_ERR_CARD_WRITE_PROTECTED, N_("Card write protected")},
	{KNC_CAM_RES_ERR_NO_SPACE_LEFT_ON_CARD, N_("No space left on card")},
	{KNC_CAM_RES_ERR_IMAGE_PROTECTED, N_("Image protected")},
	{KNC_CAM_RES_ERR_LIGHT_TOO_DARK, N_("Light too dark")},
	{KNC_CAM_RES_ERR_AUTOFOCUS, N_("Autofocus error")},
	{KNC_CAM_RES_ERR_SYSTEM, N_("System error")},
	{KNC_CAM_RES_ERR_ILLEGAL_PARAMETER, N_("Illegal parameter")},
	{KNC_CAM_RES_ERR_CMD_CANNOT_BE_CANCELLED,
				N_("Command cannot be cancelled")},
	{KNC_CAM_RES_ERR_LOCALIZATION_DATA_TOO_LONG,
				N_("Localization data too long")},
	{KNC_CAM_RES_ERR_LOCALIZATION_DATA_CORRUPT,
				N_("Localization data corrupt")},
	{KNC_CAM_RES_ERR_UNSUPPORTED_CMD, N_("Unsupported command")},
	{KNC_CAM_RES_ERR_OTHER_CMD_EXECUTING, N_("Other command executing")},
	{KNC_CAM_RES_ERR_CMD_ORDER_ERROR, N_("Command order error")},
	{KNC_CAM_RES_ERR_UNKNOWN_ERROR, N_("Unknown error")},
	{KNC_CAM_RES_ERR_INVALID_SIZE_OF_IMAGE_ID,
				N_("Invalid size of image ID")},
	{0, NULL}
};

const char *
knc_cam_res_name (KncCamRes r)
{
	unsigned int i;

	for (i = 0; KncCamResNames[i].name; i++)
		if (KncCamResNames[i].r == r) break;
	return _(KncCamResNames[i].name);
}
