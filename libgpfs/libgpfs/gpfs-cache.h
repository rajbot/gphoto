#ifndef __GPFS_CACHE_H__
#define __GPFS_CACHE_H__

#include <libgpfs/gpfs-bag.h>
#include <libgpfs/gpfs-if.h>

typedef struct _GPFsCache GPFsCache;

GPFsCache *gpfs_cache_new    (void);

/* Caching interfaces */
void          gpfs_cache_if_add            (GPFsCache *, GPFsIf *);
unsigned int  gpfs_cache_if_count          (GPFsCache *);
GPFsIf       *gpfs_cache_if_get            (GPFsCache *, unsigned int);
void          gpfs_cache_if_remove         (GPFsCache *, GPFsIf *);
void          gpfs_cache_if_set_limit_read (GPFsCache *, GPFsIf *,
					    unsigned int);
void          gpfs_cache_if_set_limit_prop (GPFsCache *, GPFsIf *,
					    unsigned int);
unsigned int  gpfs_cache_if_get_limit_read (GPFsCache *, GPFsIf *);
unsigned int  gpfs_cache_if_get_limit_prop (GPFsCache *, GPFsIf *);

/* Caching property bags */
void          gpfs_cache_add_bag           (GPFsCache *, GPFsBag *);
unsigned int  gpfs_cache_count_bag         (GPFsCache *);
GPFsBag      *gpfs_cache_get_bag           (GPFsCache *, unsigned int);
void          gpfs_cache_remove_bag        (GPFsCache *, unsigned int);

#endif
