#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include "defs.h"
#include "transmission.h"
#include "open.h"
#include "error.h"

struct termios newt;
struct termios oldt;

int qm100_open(const char *devname)
{
  int serialdev;
  qm100_packet_block packet;
  char cmd_init[QM100_INIT_LEN]=QM100_INIT;

  serialdev = open(devname, O_RDWR|O_NOCTTY);
  if (serialdev < 0) qm100_error(serialdev, "Cannot open device");

  if (tcgetattr(serialdev, &oldt) < 0) qm100_error(serialdev, "tcgetattr");
  memcpy((char *)&newt,(char *)&oldt, sizeof(struct termios));

  newt.c_cflag |= CS8;
  newt.c_iflag &= ~(IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK|ISTRIP|INLCR);
  newt.c_iflag &= ~(IGNCR|ICRNL|IXON|IXOFF|IXANY|IMAXBEL);
  newt.c_oflag &= ~(OPOST);
  newt.c_lflag &= ~(ISIG|ICANON);
  newt.c_cc[VMIN] = 1;
  newt.c_cc[VTIME] = 0;
  
  cfsetospeed(&newt, B9600);
  cfsetispeed(&newt, B9600);

  if (tcsetattr(serialdev, TCSANOW, &newt) < 0) qm100_error(serialdev, "Serial speed change problem");
  packet = qm100_transmit(serialdev, cmd_init, sizeof(cmd_init));

  return serialdev;
}







