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
  packet = qm100_transmit(serialdev, cmd_setpic, sizeof(cmd_setpic));

  /* Set Flash */

  cmd_setpic[4]=0x00;
  cmd_setpic[5]=0x90;
  cmd_setpic[6]=FLASH_AUTO;
  packet = qm100_transmit(serialdev, cmd_setpic, sizeof(cmd_setpic));

  /* Set Exposure */

  cmd_setpic[4]=0x02;
  cmd_setpic[5]=0x80;
  cmd_setpic[6]=0x00;
  packet = qm100_transmit(serialdev, cmd_setpic, sizeof(cmd_setpic));

  /* Set Focus */

  cmd_setpic[4]=0x02;
  cmd_setpic[5]=0x90;
  cmd_setpic[6]=FOCUS_AUTO;
  packet = qm100_transmit(serialdev, cmd_setpic, sizeof(cmd_setpic));

  /* Take Picture */

  packet = qm100_transmit(serialdev, cmd_takepic, sizeof(cmd_takepic));

}




