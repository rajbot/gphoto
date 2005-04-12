#include <stdio.h>
#include <stdlib.h>

#include <dlt-lib.h>
#include <config.h>

int main(const int argc, const char *argv[])
{
  int ret;
  printf("dlt-bin (" PACKAGE_NAME ") " PACKAGE_VERSION "\n");
  dlt_init();
  ret = dlt_test(argc, argv,
		 (getenv("LOAD_EXPLICIT_FILES") != NULL),
		 (getenv("TRY_NON_MODULES") != NULL));
  dlt_exit();
  printf("dlt-bin: Finished.\n");
  return ret;
}
