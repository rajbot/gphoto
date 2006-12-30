#include "config.h"
#include "gpfs-i18n.h"
#include "gpfs-val-type.h"

static struct {
	GPFsValType t;
	const char *name;
} GPFsValTypeNames[] = {
	{GPFS_VAL_TYPE_BOOL  , N_("Boolean")},
	{GPFS_VAL_TYPE_STRING, N_("String")},
	{GPFS_VAL_TYPE_UINT  , N_("Unsigned integer")},
	{GPFS_VAL_TYPE_INT   , N_("Integer")},
	{0, NULL}
};

const char *
gpfs_val_type_get_name (GPFsValType t)
{
	unsigned int i;

	for (i = 0; GPFsValTypeNames[i].name; i++)
		if (GPFsValTypeNames[i].t == t)
			break;
	return _(GPFsValTypeNames[i].name);
}
