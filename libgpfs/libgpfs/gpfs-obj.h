#ifndef __GPFS_OBJ_H__
#define __GPFS_OBJ_H__

typedef struct _GPFsObjPrivate GPFsObjPrivate;
typedef struct _GPFsObj        GPFsObj;
struct _GPFsObj {
	void (* f_free) (GPFsObj *);

	GPFsObjPrivate *priv;
};

#include <libgpfs/gpfs-err.h>
#include <libgpfs/gpfs-bag.h>

#define GPFS_OBJ(o) ((GPFsObj *)(o))

GPFsObj *gpfs_obj_new   (unsigned int size);
void     gpfs_obj_ref   (GPFsObj *);
void     gpfs_obj_unref (GPFsObj *);

const char  *gpfs_obj_get_name   (GPFsObj *, GPFsErr *);
void         gpfs_obj_set_name   (GPFsObj *, GPFsErr *, const char *);

unsigned int gpfs_obj_bag_count  (GPFsObj *, GPFsErr *);
GPFsBag     *gpfs_obj_bag_get    (GPFsObj *, GPFsErr *, unsigned int i);
void         gpfs_obj_bag_remove (GPFsObj *, GPFsErr *, unsigned int i);
void         gpfs_obj_bag_add    (GPFsObj *, GPFsErr *, GPFsBag *);

/* Functions for getting/setting the object's name */
typedef void        (* GPFsObjFuncSetName) (GPFsObj *, GPFsErr *, const char *,
					    void *);
typedef const char *(* GPFsObjFuncGetName) (GPFsObj *, GPFsErr *, void *);
void gpfs_obj_set_func_get_name (GPFsObj *, GPFsObjFuncGetName  , void * );
void gpfs_obj_get_func_get_name (GPFsObj *, GPFsObjFuncGetName *, void **);
void gpfs_obj_set_func_set_name (GPFsObj *, GPFsObjFuncSetName  , void * );
void gpfs_obj_get_func_set_name (GPFsObj *, GPFsObjFuncSetName *, void **);

/* Functions for counting property bags */
typedef unsigned int (* GPFsObjFuncBagCount) (GPFsObj *, GPFsErr *, void *);
void gpfs_obj_set_func_bag_count  (GPFsObj *, GPFsObjFuncBagCount  , void * );
void gpfs_obj_get_func_bag_count  (GPFsObj *, GPFsObjFuncBagCount *, void **);

/* Functions for getting, adding and removing property bags */
typedef GPFsBag  *(* GPFsObjFuncBagGet ) (GPFsObj *, GPFsErr *, unsigned int,
					  void *);
typedef void      (* GPFsObjFuncBagAdd ) (GPFsObj *, GPFsErr *, GPFsBag *,
					  void *);
typedef void      (* GPFsObjFuncBagRemove ) (GPFsObj *, GPFsErr *,
					     unsigned int, void *);
void gpfs_obj_set_func_bag_get    (GPFsObj *, GPFsObjFuncBagGet     , void * );
void gpfs_obj_get_func_bag_get    (GPFsObj *, GPFsObjFuncBagGet *   , void **);
void gpfs_obj_set_func_bag_add    (GPFsObj *, GPFsObjFuncBagAdd     , void * );
void gpfs_obj_get_func_bag_add    (GPFsObj *, GPFsObjFuncBagAdd *   , void **);
void gpfs_obj_set_func_bag_remove (GPFsObj *, GPFsObjFuncBagRemove  , void  *);
void gpfs_obj_get_func_bag_remove (GPFsObj *, GPFsObjFuncBagRemove *, void **);

#endif
