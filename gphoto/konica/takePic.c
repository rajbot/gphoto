#include "defs.h"
#include "transmission.h"

void qm100_takePic(int serialdev)
{
  unsigned char cmd_setpic[QM100_SETPIC_LEN]=QM100_SETPIC;
  unsigned char cmd_takepic[QM100_TAKEPIC_LEN]=QM100_TAKEPIC;
  qm100_packet_block packet;
  
  /* Set Quality */

  cmd_setpic[4]=0x00;
  cmd_setpic[5]=0x80;
  cmd_setpic[6]=QUALITY_FINE;
  qm100_transmit(serialdev, cmd_setpic, sizeof(cmd_setpic), &packet);

  /* Set Flash */

  cmd_setpic[4]=0x00;
  cmd_setpic[5]=0x90;
  cmd_setpic[6]=FLASH_AUTO;
  qm100_transmit(serialdev, cmd_setpic, sizeof(cmd_setpic), &packet);

  /* Set Exposure */

  cmd_setpic[4]=0x02;
  cmd_setpic[5]=0x80;
  cmd_setpic[6]=0x00;
  qm100_transmit(serialdev, cmd_setpic, sizeof(cmd_setpic), &packet);

  /* Set Focus */

  cmd_setpic[4]=0x02;
  cmd_setpic[5]=0x90;
  cmd_setpic[6]=FOCUS_AUTO;
  qm100_transmit(serialdev, cmd_setpic, sizeof(cmd_setpic), &packet);

  /* Take Picture */

  qm100_transmit(serialdev, cmd_takepic, sizeof(cmd_takepic), &packet);

}




