#ifndef __DYNLIBTEST_H_
#define __DYNLIBTEST_H_


typedef char *(*dynlibtest_func)(const char *);


#ifdef __DYNLIBTEST_LIB__

#define FUNC_PROTO(number) \
	const char * \
	dynlibtest ## number (const char *string);

#ifndef FOO
#define FOO "foo"
#endif

#define FUNC_BODY(number) \
	const char * \
	dynlibtest ## number (const char *string) \
	{ \
		if (string) { \
			return string; \
		} \
		return "Hello, this is the lib func " #number " " FOO; \
	}

FUNC_PROTO(0)
FUNC_PROTO(1)
FUNC_PROTO(2)
FUNC_PROTO(3)
FUNC_PROTO(4)

#endif

#endif
