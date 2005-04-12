#include <dlt-mod.h>

#define pld(mod) PUBSYM(mod,public_lib_data)

char * pld(MODULE) = "<MODULE>" MODULE_STRING "</MODULE>";
char * PRISYM(MODULE,private_lib_data) = "<module>" MODULE_STRING "</module>";

FUNC_BODY(MODULE,0)
FUNC_BODY(MODULE,1)
FUNC_BODY(MODULE,2)
FUNC_BODY(MODULE,3)
FUNC_BODY(MODULE,4)
