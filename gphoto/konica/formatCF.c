#include "defs.h"
#include "transmission.h"

void qm100_formatCF(int serialdev)
{
  unsigned char cmd[QM100_FORMAT_LEN] = QM100_FORMAT;
  qm100_transmit(serialdev, cmd, sizeof(cmd));
}
