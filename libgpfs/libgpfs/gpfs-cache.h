#ifndef __GPFS_CACHE_H__
#define __GPFS_CACHE_H__

#include <libgpfs/gpfs-bag.h>
#include <libgpfs/gpfs-if.h>

typedef struct _GPFsCache GPFsCache;

GPFsCache *gpfs_cache_new    (void);

/* Caching interfaces */
void          gpfs_cache_if_add        (GPFsCache *, GPFsIf *);
unsigned int  gpfs_cache_if_count      (GPFsCache *);
GPFsIf       *gpfs_cache_if_get        (GPFsCache *, unsigned int);
void          gpfs_cache_if_remove     (GPFsCache *, GPFsIf *);
void          gpfs_cache_if_set_limit  (GPFsCache *, GPFsIf *, unsigned int);
unsigned int  gpfs_cache_if_get_limit  (GPFsCache *, GPFsIf *);

/* Caching property bags */
void          gpfs_cache_bag_add       (GPFsCache *, GPFsBag *);
unsigned int  gpfs_cache_bag_count     (GPFsCache *);
GPFsBag      *gpfs_cache_bag_get       (GPFsCache *, unsigned int);
void          gpfs_cache_bag_remove    (GPFsCache *, unsigned int);
void          gpfs_cache_bag_set_limit (GPFsCache *, GPFsBag *, unsigned int);
unsigned int  gpfs_cache_bag_get_limit (GPFsCache *, GPFsBag *);

#endif
