#include "defs.h"
#include "getPicInfo.h"
#include "transmission.h"

int qm100_getPicInfo(int serialdev, int pic, qm100_packet_block *packet)
{
  unsigned char cmd[QM100_PICINFO_LEN]=QM100_PICINFO;
  qm100_packet_block *ack;
  
  cmd[5]=(char)(pic>>8 & 0xff);
  cmd[6]=(char)(pic & 0xff);
  
  qm100_attention(serialdev);
  qm100_sendPacket(serialdev, cmd, sizeof(cmd));
  qm100_getAck(serialdev);

  qm100_getPacket(serialdev, packet);
  
  qm100_continueTransmission(serialdev);
  
  qm100_getPacket(serialdev, ack);
  
  qm100_endTransmit(serialdev);

  return(1);
}




















