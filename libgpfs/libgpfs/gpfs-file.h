#ifndef __GPFS_FILE_H__
#define __GPFS_FILE_H__

#include <libgpfs/gpfs-if.h>

typedef struct _GPFsFile GPFsFile;

GPFsFile *gpfs_file_new   (void);
void      gpfs_file_ref   (GPFsFile *);
void      gpfs_file_unref (GPFsFile *);

void         gpfs_file_add_if    (GPFsFile *, GPFsIf *);
unsigned int gpfs_file_count_ifs (GPFsFile *);
GPFsIf      *gpfs_file_get_if    (GPFsFile *, unsigned int);

#endif
