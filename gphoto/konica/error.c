#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#ifdef _CLI_
 extern struct termios oldt;
#else
 #include "../src/gphoto.h"
 #include "../src/util.h"
#endif


void qm100_error(int serialdev, char *error)
{
#ifdef _CLI_
  perror(error);
#else
  error_dialog(error);
#endif

#ifdef _CLI_
  tcsetattr(serialdev, TCSANOW, &oldt);
#endif

  close(serialdev);
  printf("\nClosed and done!\n");
  exit(1);
}




