#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "defs.h"
#include "transmission.h"
#include "../src/gphoto.h"
#include "saveThumb.h"

struct Image *qm100_saveThumb(int serialdev, char *filename, int pic)
{
  int jpgfile;
  long jpgfile_size;
  struct Image *im;

  char success=1;
  char cmd_getthumb[QM100_GETTHUMB_LEN]=QM100_GETTHUMB;
  qm100_packet_block packet;  

  cmd_getthumb[5] = (pic >> 8) & 0xff;
  cmd_getthumb[6] = (pic & 0xff);

  qm100_attention(serialdev);
  qm100_sendPacket(serialdev, cmd_getthumb, sizeof(cmd_getthumb));
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
	/* Scott was here :P */
  jpgfile = fopen(filename, "r");
  fseek(jpgfile, 0, SEEK_END);
  jpgfile_size = ftell(jpgfile);
  rewind(jpgfile);
  im = (struct Image*)malloc(sizeof(struct Image));
  im->image = (char *)malloc(sizeof(char)*jpgfile_size);
  strcpy(im->image_type, "jpg");
  im->image_size = (int)jpgfile_size;
  im->image_info_size = 0;
	/* End of the scott hack */
  return (im);
}


