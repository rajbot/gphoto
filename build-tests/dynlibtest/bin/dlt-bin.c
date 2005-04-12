#include <stdio.h>

#include <dlt-lib.h>
#include <config.h>

int main(const int argc, const char *argv[])
{
  int ret;
  printf("dlt-bin (" PACKAGE_NAME ") " PACKAGE_VERSION "\n");
  dlt_init();
  ret = dlt_test(argc, argv);
  dlt_exit();
  printf("dlt-bin: Finished.\n");
  return ret;
}
