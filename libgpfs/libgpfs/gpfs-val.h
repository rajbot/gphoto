#ifndef __GPFS_VAL_H__
#define __GPFS_VAL_H__

#include <libgpfs/gpfs-val-type.h>

typedef struct _GPFsVal GPFsVal;

struct _GPFsVal {
	GPFsValType t;
	union {
		char         v_bool : 1;
		int          v_int;
		unsigned int v_uint;
		char        *v_string;
	} v;
};

void gpfs_val_init  (GPFsVal *);
void gpfs_val_clear (GPFsVal *);
void gpfs_val_copy  (GPFsVal *, GPFsVal *);

#endif
