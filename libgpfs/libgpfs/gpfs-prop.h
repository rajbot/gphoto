#ifndef __GPFS_PROP_H__
#define __GPFS_PROP_H__

typedef struct _GPFsProp     GPFsProp;

#include <libgpfs/gpfs-val.h>
#include <libgpfs/gpfs-err.h>
#include <libgpfs/gpfs-obj.h>

typedef enum {
	GPFS_ALT_TYPE_NONE,
	GPFS_ALT_TYPE_VALS,
	GPFS_ALT_TYPE_RANGE
} GPFsAltType;

typedef struct _GPFsPropPriv GPFsPropPriv;

struct _GPFsProp {
	GPFsObj parent;
	GPFsPropPriv *priv;

	/* Alternatives */
	GPFsAltType t;
	union {

		/* One or more values */
		struct {
			GPFsVal     *vals;
			unsigned int vals_count;
		} vals;

		/* A range of values */
		struct {
			float min, max, incr;
		} range;
	} alt;
};

GPFsProp *gpfs_prop_new   (const char *id, const char *name,
			   const char *description, GPFsVal *);

const char *gpfs_prop_get_id          (GPFsProp *);
const char *gpfs_prop_get_name        (GPFsProp *);
const char *gpfs_prop_get_description (GPFsProp *);

void      gpfs_prop_get_val (GPFsProp *, GPFsErr *, GPFsVal *);
void      gpfs_prop_set_val (GPFsProp *, GPFsErr *, GPFsVal *);

typedef void (* GPFsPropFuncSetVal) (GPFsProp *, GPFsErr *, GPFsVal *, void *);
void gpfs_prop_set_func_set_val (GPFsProp *, GPFsPropFuncSetVal  , void * );
void gpfs_prop_get_func_set_val (GPFsProp *, GPFsPropFuncSetVal *, void **);

void gpfs_prop_dump  (GPFsProp *);

#endif
