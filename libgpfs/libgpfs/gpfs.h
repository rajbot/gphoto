#ifndef __GP_FS_H__
#define __GP_FS_H__

#include <libgpfs/gpfs-err.h>
#include <libgpfs/gpfs-file.h>

typedef struct _GPFs GPFs;
typedef unsigned int (* GPFsFuncCountFiles)   (GPFs *, GPFsErr *, const char *,
					       void *);
typedef unsigned int (* GPFsFuncCountFolders) (GPFs *, GPFsErr *, const char *,
					       void *);
typedef GPFsFile *     (* GPFsFuncGetFile)    (GPFs *, GPFsErr *, const char *,
					       unsigned int, void *);
typedef char *         (* GPFsFuncGetFolder)  (GPFs *, GPFsErr *, const char *,
					       unsigned int, void *);

GPFs *gpfs_new   (void);
void  gpfs_ref   (GPFs *);
void  gpfs_unref (GPFs *);

/* Listing files */
unsigned int  gpfs_count_files (GPFs *, GPFsErr *, const char *);
GPFsFile     *gpfs_get_file    (GPFs *, GPFsErr *, const char *, unsigned int);

/* Listing folders */
unsigned int  gpfs_count_folders (GPFs *, GPFsErr *, const char *);
char         *gpfs_get_folder    (GPFs *, GPFsErr *, const char *,
				  unsigned int);

void gpfs_set_func_count_files   (GPFs *, GPFsFuncCountFiles    , void * );
void gpfs_get_func_count_files   (GPFs *, GPFsFuncCountFiles *  , void **);
void gpfs_set_func_get_file      (GPFs *, GPFsFuncGetFile       , void * );
void gpfs_get_func_get_file      (GPFs *, GPFsFuncGetFile *     , void **);
void gpfs_set_func_count_folders (GPFs *, GPFsFuncCountFolders  , void * );
void gpfs_get_func_count_folders (GPFs *, GPFsFuncCountFolders *, void **);
void gpfs_set_func_get_folder    (GPFs *, GPFsFuncGetFolder     , void * );
void gpfs_get_func_get_folder    (GPFs *, GPFsFuncGetFolder *   , void **);

#endif /* GP_FS_H__ */
