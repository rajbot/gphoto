#ifndef __GPFS_CACHE_H__
#define __GPFS_CACHE_H__

#include <libgpfs/gpfs.h>
#include <libgpfs/gpfs-if.h>

typedef struct _GPFsCache GPFsCache;

GPFsCache *gpfs_cache_new    (void);
void       gpfs_cache_ref    (GPFsCache *);
void       gpfs_cache_unref  (GPFsCache *);

/* Caching interfaces */
void          gpfs_cache_add_if            (GPFsCache *, GPFsIf *);
unsigned int  gpfs_cache_count_if          (GPFsCache *);
GPFsIf       *gpfs_cache_get_if            (GPFsCache *, unsigned int);
void          gpfs_cache_remove_if         (GPFsCache *, GPFsIf *);
void          gpfs_cache_if_set_limit_read (GPFsCache *, GPFsIf *,
					    unsigned int);
void          gpfs_cache_if_set_limit_info (GPFsCache *, GPFsIf *,
					    unsigned int);
unsigned int  gpfs_cache_if_get_limit_read (GPFsCache *, GPFsIf *);
unsigned int  gpfs_cache_if_get_limit_info (GPFsCache *, GPFsIf *);

/* Caching filesystems (structure only) */
void          gpfs_cache_add_fs    (GPFsCache *, GPFs *);
unsigned int  gpfs_cache_count_fs  (GPFsCache *);
GPFs         *gpfs_cache_get_fs    (GPFsCache *, unsigned int);
void          gpfs_cache_remove_fs (GPFsCache *, GPFs *);

#endif
