#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "defs.h"
#include "transmission.h"
#include "savePic.h"

int qm100_savePic(int serialdev, char *filename, int pic)
{
  int jpgfile;
  char success=1;
  char cmd_getpic[QM100_GETPIC_LEN]=QM100_GETPIC;
  //  char cmd_getpic[QM100_GETTHUMB_LEN]=QM100_GETTHUMB;
  qm100_packet_block packet;  

  cmd_getpic[5] = (pic >> 8) & 0xff;
  cmd_getpic[6] = (pic & 0xff);

  qm100_attention(serialdev);
  qm100_sendPacket(serialdev, cmd_getpic, sizeof(cmd_getpic));
  qm100_getAck(serialdev);
  packet = qm100_getPacket(serialdev);

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
	  qm100_continueTransmission(serialdev);
          packet = qm100_getPacket(serialdev);
          write(jpgfile, packet.packet, packet.packet_len);
        }

      close(jpgfile);
    }
  qm100_endTransmit(serialdev);
  return success;
}
