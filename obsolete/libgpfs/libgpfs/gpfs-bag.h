#ifndef __GPFS_BAG_H__
#define __GPFS_BAG_H__

typedef struct _GPFsBag GPFsBag;

#include <libgpfs/gpfs-prop.h>
#include <libgpfs/gpfs-err.h>

GPFsBag    *gpfs_bag_new      (void);

/* Properties */
unsigned int gpfs_bag_prop_count    (GPFsBag *, GPFsErr *);
GPFsProp    *gpfs_bag_prop_get      (GPFsBag *, GPFsErr *, unsigned int);
void         gpfs_bag_prop_remove   (GPFsBag *, GPFsErr *, unsigned int);
void         gpfs_bag_prop_add      (GPFsBag *, GPFsErr *, GPFsProp *);
void         gpfs_bag_prop_add_impl (GPFsBag *, GPFsProp *);

/* Bags */
unsigned int gpfs_bag_bag_count     (GPFsBag *, GPFsErr *);
GPFsBag     *gpfs_bag_bag_get       (GPFsBag *, GPFsErr *, unsigned int);
void         gpfs_bag_bag_remove    (GPFsBag *, GPFsErr *, unsigned int);
void         gpfs_bag_bag_add       (GPFsBag *, GPFsErr *, GPFsBag *);
void         gpfs_bag_bag_add_impl  (GPFsBag *, GPFsBag *);

/* Counting */
typedef unsigned int (* GPFsBagFuncCount) (GPFsBag *, GPFsErr *, void *);
void gpfs_bag_set_func_bag_count   (GPFsBag *, GPFsBagFuncCount  , void  *);
void gpfs_bag_get_func_bag_count   (GPFsBag *, GPFsBagFuncCount *, void **);
void gpfs_bag_set_func_prop_count  (GPFsBag *, GPFsBagFuncCount  , void  *);
void gpfs_bag_get_func_prop_count  (GPFsBag *, GPFsBagFuncCount *, void **);

/* Getting */
typedef GPFsProp * (* GPFsBagFuncPropGet) (GPFsBag *, GPFsErr *e,
					   unsigned int, void *);
typedef GPFsBag  * (* GPFsBagFuncBagGet ) (GPFsBag *, GPFsErr *e,
					   unsigned int, void *);
void gpfs_bag_set_func_prop_get    (GPFsBag *, GPFsBagFuncPropGet  , void  *);
void gpfs_bag_get_func_prop_get    (GPFsBag *, GPFsBagFuncPropGet *, void **);
void gpfs_bag_set_func_bag_get     (GPFsBag *, GPFsBagFuncBagGet   , void  *);
void gpfs_bag_get_func_bag_get     (GPFsBag *, GPFsBagFuncBagGet  *, void **);

/* Adding */
typedef void (* GPFsBagFuncBagAdd ) (GPFsBag *, GPFsErr *, GPFsBag  *, void *);
typedef void (* GPFsBagFuncPropAdd) (GPFsBag *, GPFsErr *, GPFsProp *, void *);
void gpfs_bag_set_func_bag_add     (GPFsBag *, GPFsBagFuncBagAdd   , void  *);
void gpfs_bag_get_func_bag_add     (GPFsBag *, GPFsBagFuncBagAdd  *, void **);
void gpfs_bag_set_func_prop_add    (GPFsBag *, GPFsBagFuncPropAdd  , void  *);
void gpfs_bag_get_func_prop_add    (GPFsBag *, GPFsBagFuncPropAdd *, void **);

/* Removing */
typedef void (* GPFsBagFuncRemove) (GPFsBag *, GPFsErr *, unsigned int, void *);
void gpfs_bag_set_func_remove_prop (GPFsBag *, GPFsBagFuncRemove  , void  *);
void gpfs_bag_get_func_remove_prop (GPFsBag *, GPFsBagFuncRemove *, void **);
void gpfs_bag_set_func_remove_bag  (GPFsBag *, GPFsBagFuncRemove  , void  *);
void gpfs_bag_get_func_remove_bag  (GPFsBag *, GPFsBagFuncRemove *, void **);

/* For debugging purposes */
void gpfs_bag_dump (GPFsBag *);

#endif
