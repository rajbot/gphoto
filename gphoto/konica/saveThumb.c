#include "qm100.h"

void qm100_saveThumb(int serialdev, char *filename, int pic, void (*progress)(void))
{
   int jpgfile;
   
   char success=1;
   char cmd[QM100_GETTHUMB_LEN]=QM100_GETTHUMB;
   qm100_packet_block packet;
   
   cmd[5] = (pic >> 8) & 0xff;
   cmd[6] = (pic & 0xff);

   qm100_transmit(serialdev, cmd, sizeof(cmd), &packet, "GetThumb");
   if (packet.packet_len == 4)
      {
      success = 0; /* No pic there! */
      }
   else
      {
      jpgfile = open(filename, O_RDWR|O_CREAT|O_EXCL, 0700);
      write(jpgfile, packet.packet, packet.packet_len);
      
      while (packet.transmission_continues)
         {
         if (progress)
            progress();
         qm100_continueTransmission(serialdev, "GetThumb");
         qm100_getPacket(serialdev, &packet);
         write(jpgfile, packet.packet, packet.packet_len);
         }
      close(jpgfile);     
      }
   qm100_endTransmit(serialdev, "GetThumb");
   qm100_sendNullCmd(serialdev);
}
