#include <unistd.h>
#include <termios.h>
#include "defs.h"
#include "transmission.h"
#include "close.h"

void qm100_close(int serialdev)
{
  unsigned char cmd_suspend[QM100_SUSPEND_LEN] = QM100_SUSPEND;

  qm100_transmit(serialdev, cmd_suspend, sizeof(cmd_suspend));
  close(serialdev);
}
