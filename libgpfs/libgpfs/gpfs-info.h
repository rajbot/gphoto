#ifndef __GPFS_INFO_H__
#define __GPFS_INFO_H__

#include <libgpfs/gpfs-info-type.h>

typedef struct _GPFsInfo GPFsInfo;
struct _GPFsInfo {
	char *id;          /* Not translated */
	char *name;        /* Translated */
	char *description; /* Translated */
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
	free ((i)->id);				\
	free ((i)->name);			\
	free ((i)->description);		\
	if ((i)->t == GPFS_INFO_TYPE_STRING)	\
		free ((i)->v.v_string);		\
}

#endif
