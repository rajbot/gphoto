#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "lowlevel.h"
#include "error.h"

char qm100_readByte(int serialdev)
{
  char byte;
  if ((read(serialdev, &byte, 1)) < -1) qm100_error(serialdev, "Cannot read from device");

#ifdef _CLI_
  if (qm100_showReadBytes) qm100_iostat("recv :", &byte, 1);
#endif

  return byte;
}

char qm100_readTimedByte(int serialdev)
{
  fd_set rfds;
  struct timeval tv;

  FD_ZERO(&rfds);
  FD_SET(serialdev, &rfds);
  tv.tv_sec=0;
  tv.tv_usec=10000;
  return (select(1+serialdev, &rfds, NULL, NULL, &tv));
}

char qm100_readCodedByte(int serialdev)
{
  char byte;
  byte=qm100_readByte(serialdev);
  if (byte==0x1b)
    {
      byte=qm100_readByte(serialdev);
      byte=(~byte & 0xff);
    }
  return byte;
}

void qm100_writeByte(int serialdev, char data)
{
  usleep(10);
  if ((write(serialdev, &data, 1)) < -1) qm100_error(serialdev, "Cannot write to device");

#ifdef _CLI_
  if (qm100_showWriteBytes) qm100_iostat("sent :", &data, 1);
#endif

}

void qm100_iostat(unsigned char *str, unsigned char *buf, int len)
{
  printf("%s [ ", str);

  if(len>0)
    {
      int p = 1;

      printf("0x%x", buf[0]);

      while(p<len)
          printf(", 0x%x", (unsigned char)buf[p++]);
    }
  printf(" ]\n");
}
