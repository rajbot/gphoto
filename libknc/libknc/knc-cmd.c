#include <config.h>
#include "knc-cmd.h"
#include "knc-i18n.h"

static struct {
	KncCmd cmd;
	const char *name;
	unsigned int data;
} KncCmds[] = {
	{KNC_ERASE_IMAGE, N_("Erase image"), 0},
	{KNC_FORMAT_MEMORY_CARD, N_("Format memory card"), 0},
	{KNC_ERASE_ALL, N_("Erase all images"), 0},
	{KNC_SET_PROTECT_STATUS, N_("Set protect status"), 0},
	{KNC_GET_IMAGE_THUMB, N_("Get thumbnail"), 1},
	{KNC_GET_IMAGE_JPEG, N_("Get image as JPEG excluding "
				"EXIF information"), 1},
	{KNC_GET_IMAGE_INFO, N_("Get information about an image "), 1},
	{KNC_GET_IMAGE_EXIF, N_("Get image as JPEG including "
				"EXIF information"), 1},
	{KNC_GET_PREVIEW, N_("Capture a preview"), 1},
	{KNC_GET_IO_PREF, N_("Get io-capabilities"), 0},
	{KNC_GET_INFO, N_("Get information"), 0},
	{KNC_GET_STATUS, N_("Get status"), 0},
	{KNC_GET_DATE_AND_TIME, N_("Get date and time"), 0},
	{KNC_GET_PREFERENCES, N_("Get preferences"), 0},
	{KNC_SET_IO_PREF, N_("Set io-capabilities"), 0},
	{KNC_SET_DATE_AND_TIME, N_("Set date and time"), 0},
	{KNC_SET_PREFERENCE, N_("Set preferences"), 0},
	{KNC_RESET_PREFERENCES, N_("Reset preferences"), 0},
	{KNC_TAKE_PICTURE, N_("Take picture"), 1},
	{KNC_LOCALIZATION, N_("Adjust country specific settings"), 0},
	{KNC_CANCEL, N_("Cancel"), 0},
	{0, NULL, 0}
};

const char *
knc_cmd_name (KncCmd cmd)
{
	unsigned int i;

	for (i = 0; KncCmds[i].name; i++) if (KncCmds[i].cmd == cmd) break;
	return _(KncCmds[i].name);
}

unsigned int
knc_cmd_data (KncCmd cmd)
{
	unsigned int i;

	for (i = 0; KncCmds[i].name; i++) if (KncCmds[i].cmd == cmd) break;

	return KncCmds[i].data;
}
