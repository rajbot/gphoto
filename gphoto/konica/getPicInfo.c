#include "defs.h"
#include "getPicInfo.h"
#include "transmission.h"

qm100_packet_block qm100_getPicInfo(int serialdev, int pic)
{
  unsigned char cmd[QM100_PICINFO_LEN]=QM100_PICINFO;
  qm100_packet_block packet, ack;
  
  cmd[5]=(char)(pic>>8 & 0xff);
  cmd[6]=(char)(pic & 0xff);
  
  qm100_attention(serialdev);
  qm100_sendPacket(serialdev, cmd, sizeof(cmd));
  qm100_getAck(serialdev);

  packet = qm100_getPacket(serialdev);
  
  qm100_continueTransmission(serialdev);
  
  ack = qm100_getPacket(serialdev);
  
  qm100_endTransmit(serialdev);
 
  return (packet);
}




















