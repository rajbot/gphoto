#include "defs.h"
#include "transmission.h"
#include "erasePic.h"

void qm100_erasePic(int serialdev, int pic)
{
  unsigned char cmd[QM100_ERASE_LEN] = QM100_ERASE;
  cmd[5] = (pic >> 8) & 0xff;
  cmd[6] = pic & 0xff;
  qm100_transmit(serialdev, cmd, sizeof(cmd));
}
