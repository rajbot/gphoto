#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

#include "defs.h"
#include "setSpeed.h"
#include "transmission.h"
#include "error.h"

void qm100_setSpeed(int serialdev, int speed)
{
  qm100_packet_block packet;
  int reg;
  unsigned char cmd_speed[QM100_SPEED_LEN]=QM100_SPEED;
  unsigned char cmd_init[QM100_INIT_LEN]=QM100_INIT;

  newt.c_cflag |= CS8;
  newt.c_iflag &= ~(IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK|ISTRIP|INLCR);
  newt.c_iflag &= ~(IGNCR|ICRNL|IXON|IXOFF|IUCLC|IXANY|IMAXBEL);
  newt.c_oflag &= ~(OPOST);
  newt.c_lflag &= ~(ISIG|ICANON);
  newt.c_cflag &= ~(XCASE);
  newt.c_cc[VMIN] = 255;
  newt.c_cc[VTIME] = 5;

  switch (speed)
    {
    case B9600:
      reg = 0x020;
      break;
    case B19200:
      reg = 0x040;
      break;
    case B38400:
      reg = 0x080;
      break;
    case B57600:
      reg = 0x100;
      break;
    case B115200:
      reg = 0x200;
      break;
    }
  cmd_speed[4]=(reg & 0xff);
  cmd_speed[5]=((reg>>8) & 0xff);

  qm100_transmit(serialdev, cmd_speed, sizeof(cmd_speed), &packet);

  cfsetispeed(&newt, speed);
  cfsetospeed(&newt, speed);

  if (tcsetattr(serialdev, TCSANOW, &newt) < 0) qm100_error(serialdev, "Serial speed change problem");

  qm100_transmit(serialdev, cmd_init, sizeof(cmd_init), &packet);
}


