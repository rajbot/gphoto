#ifndef __GP_FS_H__
#define __GP_FS_H__

#include <libgpfs/gpfs-err.h>
#include <libgpfs/gpfs-file.h>
#include <libgpfs/gpfs-cache.h>

typedef struct _GPFs GPFs;
typedef GPFsFile *     (* GPFsFuncGetFile)    (GPFs *, GPFsErr *, const char *,
					       unsigned int, void *);
typedef char *         (* GPFsFuncGetFolder)  (GPFs *, GPFsErr *, const char *,
					       unsigned int, void *);

GPFs *gpfs_new (void);

/* Listing files */
unsigned int  gpfs_file_count (GPFs *, GPFsErr *, const char *);
GPFsFile     *gpfs_file_get    (GPFs *, GPFsErr *, const char *, unsigned int);

/* Listing folders */
unsigned int  gpfs_folder_count (GPFs *, GPFsErr *, const char *);
char         *gpfs_folder_get   (GPFs *, GPFsErr *, const char *,
				 unsigned int);

/* Caching */
GPFsCache    *gpfs_get_cache     (GPFs *);

typedef unsigned int (* GPFsFuncCount) (GPFs *, GPFsErr *, const char *,
					void *);
void gpfs_set_func_file_count   (GPFs *, GPFsFuncCount         , void * );
void gpfs_get_func_file_count   (GPFs *, GPFsFuncCount *       , void **);
void gpfs_set_func_folder_count (GPFs *, GPFsFuncCount         , void * );
void gpfs_get_func_folder_count (GPFs *, GPFsFuncCount *       , void **);
void gpfs_set_func_file_get     (GPFs *, GPFsFuncGetFile       , void * );
void gpfs_get_func_file_get     (GPFs *, GPFsFuncGetFile *     , void **);
void gpfs_set_func_folder_get   (GPFs *, GPFsFuncGetFolder     , void * );
void gpfs_get_func_folder_get   (GPFs *, GPFsFuncGetFolder *   , void **);

#endif /* GP_FS_H__ */
