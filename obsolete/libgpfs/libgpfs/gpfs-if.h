#ifndef __GP_IF_H__
#define __GP_IF_H__

#include <libgpfs/gpfs-err.h>
#include <libgpfs/gpfs-prop.h>

typedef struct _GPFsIf GPFsIf;
typedef void         (* GPFsIfFuncReadCb)    (GPFsIf *, GPFsErr *,
					      const char *, unsigned int,
					      void *);
typedef void         (* GPFsIfFuncRead)      (GPFsIf *, GPFsErr *,
					      GPFsIfFuncReadCb, void *, void *);

GPFsIf *gpfs_if_new   (void);

const char *gpfs_if_get_name (GPFsIf *);

void  gpfs_if_read              (GPFsIf *, GPFsErr *, GPFsIfFuncReadCb, void *);

void gpfs_if_set_func_read       (GPFsIf *, GPFsIfFuncRead       , void * );
void gpfs_if_get_func_read       (GPFsIf *, GPFsIfFuncRead *     , void **);

#endif
