#ifndef __DYNLIBTEST_H_
#define __DYNLIBTEST_H_


typedef char *(*dynlibtest_func)(const char *);


#ifdef __DYNLIBTEST_LIB__

/* not sure whether we need this when using libltdl */
#ifndef WIN32
#define __declspec(foo)
#endif

/**
 * We NEED this to name our exported symbols.
 * See the libtool.info chapter on "Modules for libltdl".
 **/
#ifndef MODULE
#error MODULE is undefined!
#endif

#define __MODULE_STRING(mod) #mod
#define _MODULE_STRING(mod) __MODULE_STRING(mod)
#define MODULE_STRING _MODULE_STRING(MODULE)

#define foerks(prefix,ident) prefix ## ident

#define me(mod,ident) foerks(mod ## _LTX_, ident)

/**
 * Define prototype for test function
 **/
#define FUNC_PROTO(mod,number) \
	const char * \
	me(mod, dynlibtest ## number) (const char *string);

/**
 * Define body for test function
 **/
#define FUNC_BODY(mod,number) \
	const char * \
	me(mod, dynlibtest ## number) (const char *string) \
	{ \
		if (string) { \
			return string; \
		} \
		return "Hello, this is the lib func " \
			#number " " _MODULE_STRING(mod); \
	}

FUNC_PROTO(MODULE,0)
FUNC_PROTO(MODULE,1)
FUNC_PROTO(MODULE,2)
FUNC_PROTO(MODULE,3)
FUNC_PROTO(MODULE,4)

#endif /* __DYNLIBTEST_LIB__ */

#endif /* !__DYNLIBTEST_H_ */
