#ifndef __GPFS_ERR_TYPE_H__
#define __GPFS_ERR_TYPE_H__

typedef enum {
	GPFS_ERR_TYPE_NONE = 0,

	GPFS_ERR_TYPE_NO_MEMORY,
	GPFS_ERR_TYPE_NOT_SUPPORTED,
	GPFS_ERR_TYPE_BAD_PARAMETERS
} GPFsErrType;

const char *gpfs_err_type_get_name (GPFsErrType t);

#endif
