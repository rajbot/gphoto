#include "io.h"

void dump(char *str, char *buf, int len) 
{  
  printf("%s [ ", str);

  if(len>0) 
    {
      int p = 1;

      printf("0x%x", buf[0]);

      while(p<len)
	  printf(", 0x%x", buf[p++]);
    }
  printf(" ]\n");

}

int main(int argc, char **argv)
{
  io_link *dev;
  char buf[20];
  
  dev = io_open("/dev/ttyS1");

  io_set_baudrate(dev, 9600);

  io_recvbuffer(dev, 5);
  dump("recv: ", dev->buf, dev->buflen);

  buf[0] = 0x05;
  dump("send: ", buf, 1);
  io_sendbuffer(dev, buf, 1);
  
  io_recvbuffer(dev, 1);      
  dump("recv: ", dev->buf, dev->buflen);
  dev->buflen=0;

  buf[0] = 0x10;
  buf[1] = 0x02;
  buf[2] = 0x00;
  buf[3] = 0x09;
  buf[4] = 0x00;
  buf[5] = 0x00;
  buf[6] = 0x10;
  buf[7] = 0x03;
  buf[8] = 0x0a;
  dump("send: ", buf, 9);
  io_sendbuffer(dev, buf, 9);

  io_recvbuffer(dev, 6);
  dump("rev: ", dev->buf, dev->buflen);

  io_close(dev);

  return 0;
}
