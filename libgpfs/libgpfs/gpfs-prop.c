#include "config.h"
#include "gpfs-i18n.h"
#include "gpfs-prop.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CNN(i,e) {                                                      \
        if (!i) {                                                       \
                gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,          \
                        _("You need to supply a property."));		\
                return NULL;                                            \
        }                                                               \
}

#define CNV(i,e) {                                                      \
        if (!i) {                                                       \
                gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,          \
                            _("You need to supply a property."));	\
                return;                                                 \
        }                                                               \
}

struct _GPFsPropPriv 
{
	unsigned int id;
	char *name, *description;

	GPFsPropFuncSetVal f_set_val; void *f_data_set_val;

	GPFsVal val;
};

static void
gpfs_prop_free (GPFsObj *o)
{
	unsigned int n;
	GPFsProp *prop = (GPFsProp *) o;

	free (prop->priv->name);
	free (prop->priv->description);
	gpfs_val_clear (&prop->priv->val);
	free (prop->priv);
	switch (prop->t) {
	case GPFS_ALT_TYPE_VALS:
		for (n = 0; n < prop->alt.vals.vals_count; n++)
			gpfs_val_clear (&prop->alt.vals.vals[n]);
		free (prop->alt.vals.vals);
		break;
	default:
		break;
	}
}

GPFsProp *
gpfs_prop_new (unsigned int id, const char *name, const char *description,
	       GPFsVal v)
{
	GPFsObj *o;

	o = gpfs_obj_new (sizeof (GPFsProp));
	if (!o) return NULL;
	o->f_free = gpfs_prop_free;
	((GPFsProp *) o)->priv = malloc (sizeof (GPFsPropPriv));
	if (!((GPFsProp *) o)->priv) {free (o); return NULL;}
	memset (((GPFsProp *) o)->priv, 0, sizeof (GPFsPropPriv));

	((GPFsProp *) o)->priv->id          = id;
	((GPFsProp *) o)->priv->name        = strdup (name);
	((GPFsProp *) o)->priv->description = strdup (description);
	gpfs_val_copy (&((GPFsProp *) o)->priv->val, &v);

	return (GPFsProp *) o;
}

const char *
gpfs_prop_get_description (GPFsProp *i)
{
	return i ? i->priv->description : NULL;
}

const char *
gpfs_prop_get_name (GPFsProp *i)
{
	return i ? i->priv->name : NULL;
}

unsigned int
gpfs_prop_get_id (GPFsProp *i)
{
	return i ? i->priv->id : 0;
}

void
gpfs_prop_get_val (GPFsProp *i, GPFsErr *e, GPFsVal *v)
{
	CNV(i,e);

	gpfs_val_copy (v, &i->priv->val);
}

void
gpfs_prop_set_val (GPFsProp *i, GPFsErr *e, GPFsVal *v)
{
	CNV(i,e);

	if (!i->priv->f_set_val) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			      _("This value of this piece of proprmation "
				"cannot be changed."));
		return;
	}

	i->priv->f_set_val (i, e, v, i->priv->f_data_set_val);
}

void
gpfs_prop_set_func_set_val (GPFsProp *i, GPFsPropFuncSetVal f, void *f_data)
{
	if (!i) return;
	i->priv->f_set_val = f;
	i->priv->f_data_set_val = f_data;
}

void
gpfs_prop_get_func_set_val (GPFsProp *i, GPFsPropFuncSetVal *f, void **f_data)
{
	if (!i) return;
	if (f) *f = i->priv->f_set_val;
	if (f_data) *f_data = i->priv->f_data_set_val;
}

void
gpfs_prop_dump (GPFsProp *i)
{
	GPFsVal v;
	unsigned int n;

	printf ("Property %i:\n", gpfs_prop_get_id (i));
	printf (" - Name: '%s'\n", gpfs_prop_get_name (i));
	printf (" - Description: '%s'\n", gpfs_prop_get_description (i));
	gpfs_val_init (&v);
	gpfs_prop_get_val (i, NULL, &v);
	printf (" - Type: '%s'\n", gpfs_val_type_get_name (v.t));
	switch (v.t) {
	case GPFS_VAL_TYPE_STRING:
		printf (" - Value: '%s'\n", v.v.v_string);
		break;
	case GPFS_VAL_TYPE_UINT:
		printf (" - Value: %i\n", v.v.v_uint);
		break;
	case GPFS_VAL_TYPE_INT:
		printf (" - Value: %i\n", v.v.v_int);
		break;
	case GPFS_VAL_TYPE_CHAR:
		printf (" - Value: '%c'\n", v.v.v_char);
		break;
	default:
		printf (" - Value unknown\n");
	}
	gpfs_val_clear (&v);

	/* Print alternatives */
	switch (i->t) {
	case GPFS_ALT_TYPE_VALS:
		if (i->alt.vals.vals_count)
		  printf (" - Alternatives:\n");
		else
		  printf (" - No alternatives\n");
		for (n = 0; n < i->alt.vals.vals_count; n++)
		  switch (i->alt.vals.vals[n].t) {
		  case GPFS_VAL_TYPE_STRING:
		    printf ("    * '%s'\n", i->alt.vals.vals[n].v.v_string);
		    break;
		  case GPFS_VAL_TYPE_INT:
		    printf ("    * %i\n", i->alt.vals.vals[n].v.v_int);
		    break;
		  case GPFS_VAL_TYPE_UINT:
		    printf ("    * %i\n", i->alt.vals.vals[n].v.v_uint);
		    break;
		  default:
		    printf (" - Value unknown\n");
		  }
		break;
	case GPFS_ALT_TYPE_RANGE:
		printf (" - Alternative values in range between %f and %f "
			"(step %f)\n", i->alt.range.min,
			i->alt.range.max, i->alt.range.incr);
		break;
	default:
		break;
	}
}
