#include <config.h>
#include "knc-cntrl-res.h"
#include "knc-i18n.h"

static struct {
	KncCntrlRes r;
	const char *name;
} KncCntrlResNames[] = {
	{KNC_CNTRL_OK, N_("No error")},
	{KNC_CNTRL_ERR, N_("Unspecified error")},
	{KNC_CNTRL_ERR_CANCEL, N_("Command cancelled")},
	{KNC_CNTRL_ERR_NO_MEMORY, N_("Not enough memory")},
	{KNC_CNTRL_ERR_ILLEGAL_PARAMETER, N_("Illegal parameter")},
	{0, NULL}
};

const char *
knc_cntrl_res_name (KncCntrlRes r)
{
	unsigned int i;

	for (i = 0; KncCntrlResNames[i].name; i++)
		if (KncCntrlResNames[i].r == r) break;
	return _(KncCntrlResNames[i].name);
}
