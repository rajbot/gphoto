#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "defs.h"
#include "transmission.h"
#include "../src/gphoto.h"
#include "savePic.h"

struct Image *qm100_savePic(int serialdev, char *filename, int pic)
{
  int jpgfile;
  long jpgfile_size;
  struct Image *im;

  char success=1;
  char cmd_getpic[QM100_GETPIC_LEN]=QM100_GETPIC;
  qm100_packet_block packet;  

  cmd_getpic[5] = (pic >> 8) & 0xff;
  cmd_getpic[6] = (pic & 0xff);

  qm100_attention(serialdev);
  qm100_sendPacket(serialdev, cmd_getpic, sizeof(cmd_getpic));
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

  jpgfile = fopen(filename, "r");
  fseek(jpgfile, 0, SEEK_END);
  jpgfile_size = ftell(jpgfile);
  rewind(jpgfile);
  im = (struct Image*)malloc(sizeof(struct Image));
  im->image = (char *)malloc(sizeof(char)*jpgfile_size);
  fread(im->image, (size_t)sizeof(char), (size_t)jpgfile_size, jpgfile);
  strcpy(im->image_type, "jpg");
  im->image_size = (int)jpgfile_size;
  im->image_info_size = 0;
  return (im);
}
