#ifndef __GPFS_INFO_TYPE_H__
#define __GPFS_INFO_TYPE_H__

typedef enum {
	GPFS_INFO_TYPE_INT,
	GPFS_INFO_TYPE_UINT,
	GPFS_INFO_TYPE_STRING
} GPFsInfoType;

const char *gpfs_info_type_get_name (GPFsInfoType);

#endif
