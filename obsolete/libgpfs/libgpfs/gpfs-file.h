#ifndef __GPFS_FILE_H__
#define __GPFS_FILE_H__

#include <libgpfs/gpfs-if.h>

typedef struct _GPFsFile GPFsFile;

GPFsFile *gpfs_file_new   (void);

void         gpfs_file_if_add    (GPFsFile *, GPFsIf *);
unsigned int gpfs_file_if_count  (GPFsFile *);
GPFsIf      *gpfs_file_if_get    (GPFsFile *, unsigned int);
void         gpfs_file_if_remove (GPFsFile *, unsigned int);

#endif
