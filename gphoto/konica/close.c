#include "qm100.h"

void qm100_close(int serialdev)
{
  unsigned char cmd_suspend[QM100_SUSPEND_LEN] = QM100_SUSPEND;
  qm100_packet_block packet;

  qm100_setSpeed(serialdev, B9600);
  qm100_transmit(serialdev, cmd_suspend, sizeof(cmd_suspend), &packet, "Close");
  close(serialdev);
}

