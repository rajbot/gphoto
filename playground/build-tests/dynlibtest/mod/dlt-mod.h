#ifndef __DLT_MOD_H__
#define __DLT_MOD_H__


#include <dlt-lib.h>


/* not sure whether we need this when using libltdl */
#ifndef WIN32
#define __declspec(foo)
#endif

/**
 * We NEED this to name our exported symbols.
 * See the libtool.info chapter on "Modules for libltdl".
 **/
#ifdef MODULE

#define __MODULE_STRING(mod) #mod
#define _MODULE_STRING(mod) __MODULE_STRING(mod)
#define MODULE_STRING _MODULE_STRING(MODULE)

#define foerks(prefix,ident) prefix ## ident
#define foerks2(prefix,ident) foerks(_ ## prefix,ident)

#define __PUBSYM(mod,ident) foerks(mod ## _LTX_, ident)
#define __PRISYM(mod,ident) foerks2(mod ## _, ident)

#define _PUBSYM(mod,ident) __PUBSYM(mod,ident)
#define _PRISYM(mod,ident) __PRISYM(mod,ident)

#else

/* #warning MODULE is undefined! */

#define MODULE ("dummy module")
#define _MODULE_STRING(mod) "undefined"
#define _PUBSYM(mod,ident) ident
#define _PRISYM(mod,ident) ident

#endif

#define MODULE_STRING _MODULE_STRING(MODULE)
#define PUBSYM(ident) _PUBSYM(MODULE,ident)
#define PRISYM(ident) _PRISYM(MODULE,ident)

/**
 * Define prototype for test function
 **/
#define FUNC_PROTO(number) \
	const char * \
	_PUBSYM(MODULE, dynlibtest ## number) (const char *string);

/**
 * Define body for test function
 **/
#define FUNC_BODY(number) \
	const char * \
	_PUBSYM(MODULE, dynlibtest ## number) (const char *string) \
	{ \
		if (string) { \
			return string; \
		} \
		dlt_mod_callback(); \
		return "Hello, this is the \"" _MODULE_STRING(MODULE) \
			"\" module function dynlibtest"#number"()"; \
	}

FUNC_PROTO(0)
FUNC_PROTO(1)
FUNC_PROTO(2)
FUNC_PROTO(3)
FUNC_PROTO(4)

#define test_func_with_mod PUBSYM(test_func_with_mod)
const char * test_func_with_mod (void);
const char * test_func_without_mod (void);

#define public_test_data PUBSYM(public_test_data)

#endif /* !__DLT_MOD_H__ */
