/*
 * The actual code for the test functions is defined in the .h
 * file and only instantiated here.
 * 
 * This is to create multiple test modules from the same source.
 * 
 * Production modules will handle this differently, of course.
 *
 * FIXME:
 *   Link to a function in libdlt
 */

#include <dlt-mod.h>

#define private_test_data PRISYM(private_test_data)
char * public_test_data  = "<MODULE>" MODULE_STRING "</MODULE>";
char * private_test_data = "<module>" MODULE_STRING "</module>";

const char * test_func_with_mod (void)
{
  return "Just a test";
}


const char * test_func_without_mod (void)
{
  return "Just a test";
}


FUNC_BODY(0)
FUNC_BODY(1)
FUNC_BODY(2)
FUNC_BODY(3)
FUNC_BODY(4)

