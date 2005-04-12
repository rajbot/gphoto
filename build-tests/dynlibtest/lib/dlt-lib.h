#ifndef __DLT_LIB_H__
#define __DLT_LIB_H__


#ifndef assert
#define assert(expr) \
	do { \
		if (!(expr)) { \
			fprintf(stderr, \
				__FILE__ ":%d: Assertion failed: %s\n", \
				__LINE__, #expr); \
			exit(13); \
		} \
	} while (0)
#endif


#if defined(WIN32)
#  define SOEXT ".dll"
#elif defined(OS2)
#  define SOEXT ".dll"
#elif defined(DARWIN)
#  define SOEXT ".dylib"
#else
#  define SOEXT ".so"
#endif


int
dlt_init (void);

int
dlt_exit (void);

int
dlt_test (const int argc, const char *argv[]);


#endif /* ! __DLT_LIB_H__ */
