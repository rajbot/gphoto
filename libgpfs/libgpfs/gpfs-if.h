#ifndef __GP_IF_H__
#define __GP_IF_H__

#include <libgpfs/gpfs-err.h>
#include <libgpfs/gpfs-info.h>

typedef struct _GPFsIf GPFsIf;
typedef unsigned int (* GPFsIfFuncCountInfo) (GPFsIf *, GPFsErr *, void *);
typedef GPFsInfo *   (* GPFsIfFuncGetInfo  ) (GPFsIf *, GPFsErr *,
		 			      unsigned int, void *);
typedef void         (* GPFsIfFuncReadCb)    (GPFsIf *, GPFsErr *,
					      const char *, unsigned int,
					      void *);
typedef void         (* GPFsIfFuncRead)      (GPFsIf *, GPFsErr *,
					      GPFsIfFuncReadCb, void *, void *);

GPFsIf *gpfs_if_new   (const char *name);
void    gpfs_if_ref   (GPFsIf *);
void    gpfs_if_unref (GPFsIf *);

const char *gpfs_if_get_name (GPFsIf *);

/* Getting and setting pieces of information */
unsigned int gpfs_if_count_info (GPFsIf *, GPFsErr *);
GPFsInfo *   gpfs_if_get_info   (GPFsIf *, GPFsErr *, unsigned int);

/* Reading data */
void  gpfs_if_read              (GPFsIf *, GPFsErr *, GPFsIfFuncReadCb, void *);


void gpfs_if_set_func_count_info (GPFsIf *, GPFsIfFuncCountInfo  , void * );
void gpfs_if_get_func_count_info (GPFsIf *, GPFsIfFuncCountInfo *, void **);
void gpfs_if_set_func_get_info   (GPFsIf *, GPFsIfFuncGetInfo    , void * );
void gpfs_if_get_func_get_info   (GPFsIf *, GPFsIfFuncGetInfo *  , void **);
void gpfs_if_set_func_read       (GPFsIf *, GPFsIfFuncRead       , void * );
void gpfs_if_get_func_read       (GPFsIf *, GPFsIfFuncRead *     , void **);

#endif
