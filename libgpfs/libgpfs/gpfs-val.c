#include <config.h>
#include "gpfs-val.h"

#include <string.h>
#include <stdlib.h>

void
gpfs_val_init (GPFsVal *v)
{
	if (v) memset (v, 0, sizeof (GPFsVal));
}

void
gpfs_val_clear (GPFsVal *v)
{
	if (!v) return;
	switch (v->t) {
	case GPFS_VAL_TYPE_STRING:
		free (v->v.v_string);
		break;
	default:
		break;
	}
	gpfs_val_init (v);
}

void
gpfs_val_copy (GPFsVal *dest, GPFsVal *src)
{
	if (!dest) return;
	if (!src) return;
	memcpy (dest, src, sizeof (GPFsVal));
	switch (src->t) {
	case GPFS_VAL_TYPE_STRING:
		dest->v.v_string = strdup (src->v.v_string);
		break;
	default:
		break;
	}
}
