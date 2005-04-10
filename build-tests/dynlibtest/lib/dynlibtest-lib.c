#define __DYNLIBTEST_LIB__
#include <dynlibtest.h>

#ifndef WIN32
#define __declspec(foo)
#endif

__declspec(dllexport) char *lib_data;

FUNC_BODY(0)
FUNC_BODY(1)
FUNC_BODY(2)
FUNC_BODY(3)
FUNC_BODY(4)
