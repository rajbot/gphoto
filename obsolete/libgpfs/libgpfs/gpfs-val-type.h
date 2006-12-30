#ifndef __GPFS_VAL_TYPE_H__
#define __GPFS_VAL_TYPE_H__

typedef enum {
	GPFS_VAL_TYPE_NONE = 0,
	GPFS_VAL_TYPE_BOOL,
	GPFS_VAL_TYPE_INT,
	GPFS_VAL_TYPE_UINT,
	GPFS_VAL_TYPE_STRING
} GPFsValType;

const char *gpfs_val_type_get_name (GPFsValType);

#endif
