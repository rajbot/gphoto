#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "defs.h"
#include "transmission.h"
#include "../src/gphoto.h"
#include "saveThumb.h"

void qm100_saveThumb(int serialdev, char *filename, int pic)
{
  int jpgfile;

  char success=1;
  char cmd_getthumb[QM100_GETTHUMB_LEN]=QM100_GETTHUMB;
  qm100_packet_block packet;

  cmd_getthumb[5] = (pic >> 8) & 0xff;
  cmd_getthumb[6] = (pic & 0xff);

  qm100_attention(serialdev);
  qm100_sendPacket(serialdev, cmd_getthumb, sizeof(cmd_getthumb));
  qm100_getAck(serialdev);
  qm100_getPacket(serialdev, &packet);

  
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
	qm100_getPacket(serialdev, &packet);
	write(jpgfile, packet.packet, packet.packet_len);
      }
      close(jpgfile);     
    }
  qm100_endTransmit(serialdev);
}


