#include <config.h>
#include "gpfs-info-type.h"

#include "gpfs-i18n.h"

static struct {
	GPFsInfoType t;
	const char *name;
} GPFsInfoTypeNames[] = {
	{GPFS_INFO_TYPE_STRING, N_("String")},
	{GPFS_INFO_TYPE_UINT  , N_("Unsigned integer")},
	{GPFS_INFO_TYPE_INT   , N_("Integer")},
	{0, NULL}
};

const char *
gpfs_info_type_get_name (GPFsInfoType t)
{
	unsigned int i;

	for (i = 0; GPFsInfoTypeNames[i].name; i++)
		if (GPFsInfoTypeNames[i].t == t)
			break;
	return _(GPFsInfoTypeNames[i].name);
}
