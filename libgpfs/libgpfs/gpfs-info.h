#ifndef __GPFS_INFO_H__
#define __GPFS_INFO_H__

typedef enum {
	GPFS_INFO_TYPE_INT,
	GPFS_INFO_TYPE_UINT,
	GPFS_INFO_TYPE_STRING
} GPFsInfoType;

typedef struct _GPFsInfo GPFsInfo;
struct _GPFsInfo {
	char *name;
	GPFsInfoType t;
	union {
		int           v_int;
		unsigned int  v_uint;
		char         *v_string;
	} v;
};

#define gpfs_info_init(i)  {memset(i,0,sizeof(GPFsInfo));}
#define gpfs_info_clear(i) 			\
{						\
	free ((i)->name);			\
	if ((i)->t == GPFS_INFO_TYPE_STRING)	\
		free ((i)->v.v_string);		\
}

#endif
