#ifndef __DLT_MOD_H__
#define __DLT_MOD_H__


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

#define PUBSYM(mod,ident) foerks(mod ## _LTX_, ident)
#define _PRISYM(mod,ident) foerks2(mod ## _, ident)
#define PRISYM(mod,ident) _PRISYM(mod,ident)

#else

/* #warning MODULE is undefined! */

#define MODULE ("dummy module")
#define _MODULE_STRING(mod) "undefined"
#define MODULE_STRING _MODULE_STRING(MODULE)
#define PUBSYM(mod,ident) ident
#define PRISYM(mod,ident) ident

#endif


/**
 * Define prototype for test function
 **/
#define FUNC_PROTO(mod,number) \
	const char * \
	PUBSYM(mod, dynlibtest ## number) (const char *string);

/**
 * Define body for test function
 **/
#define FUNC_BODY(mod,number) \
	const char * \
	PUBSYM(mod, dynlibtest ## number) (const char *string) \
	{ \
		if (string) { \
			return string; \
		} \
		return "Hello, this is the \"" _MODULE_STRING(mod) \
			"\" module function dynlibtest"#number"()"; \
	}

FUNC_PROTO(MODULE,0)
FUNC_PROTO(MODULE,1)
FUNC_PROTO(MODULE,2)
FUNC_PROTO(MODULE,3)
FUNC_PROTO(MODULE,4)

#endif /* !__DLT_MOD_H__ */
