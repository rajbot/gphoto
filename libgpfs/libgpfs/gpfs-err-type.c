#include "config.h"
#include "gpfs-err-type.h"
#include "gpfs-i18n.h"

static struct {
	GPFsErrType t;
	const char *name;
} GPFsErrTypeTable[] = {
	{GPFS_ERR_TYPE_NONE, N_("Unspecified error")},
	{GPFS_ERR_TYPE_NO_MEMORY, N_("No memory")},
	{GPFS_ERR_TYPE_BAD_PARAMETERS, N_("Bad parameters")},
	{GPFS_ERR_TYPE_NOT_SUPPORTED, N_("Not supported")},
	{0, NULL}
};

const char *
gpfs_err_type_get_name (GPFsErrType t)
{
	unsigned int i;

	for (i = 0; GPFsErrTypeTable[i].name; i++)
		if (GPFsErrTypeTable[i].t == t)
			break;
	return _(GPFsErrTypeTable[i].name);
}
