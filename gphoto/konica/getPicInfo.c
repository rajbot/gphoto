#include "qm100.h"

int qm100_getPicInfo(int serialdev, int pic, qm100_packet_block *packet)
{
  unsigned char cmd[QM100_PICINFO_LEN]=QM100_PICINFO;
  
  cmd[5]=(char)(pic>>8 & 0xff);
  cmd[6]=(char)(pic & 0xff);

  qm100_transmit(serialdev, cmd, sizeof(cmd), packet, "GetPicInfo");
  while (packet->transmission_continues)
     {
     qm100_packet_block tpacket;
     qm100_transmit(serialdev, cmd, sizeof(cmd), &tpacket, "GetPicInfo Continuation");
     if (!tpacket.transmission_continues)
        break;
     }
  qm100_sendNullCmd(serialdev);
  return(1);
}
