/*
	The (gPhoto) I/O Library

	Modified by:
	Copyright 1999, Scott Fritzinger <scottf@unr.edu>

	Based on work by:
 	Copyright 1999, Beat Christen <spiff@longstreet.ch>
		for the toshiba gPhoto library.

	This library is covered by the LGPL.
*/

#include "io.h"

/* Serial port specific helper functions */
int io_serial_set_baudrate(io_device *dev);
static speed_t io_serial_baudconv(int rate);

/*
   Core IO library functions
   ----------------------------------------------------------------
*/

io_device *io_new (io_device_type type)
	/* Create a new IO device */
{
	io_device *dev;

	dev = (io_device *)malloc (sizeof(io_device));
	dev->type = type;

	return (dev);
}

int io_open (io_device *dev)
	/* Open a device for reading/writing */
{
  dev->device_fd = open(dev->port, O_RDWR|O_NDELAY);
  if(dev->device_fd==-1)
    {
      fprintf(stderr, "io_open: failed to open ");
      perror(dev->port);
      free(dev);
      return IO_ERROR;
    }
  return IO_OK;
}

int io_close(io_device *dev)
	/* Close the device to prevent reading/writing */
{
  if(!dev) return IO_ERROR;

  if(close(dev->device_fd)==-1)
    perror("io_close() tried closing device file descriptor");

  return IO_OK;
}

int io_reset (io_device *dev)
	/* Called whenever the settings for a particular device are
	   changed and need to update the device. */
{
	switch (dev->type) {
		case IO_DEVICE_SERIAL:
			/* Apply the baud rate change */
			io_close(dev);
			io_open(dev);
			io_serial_set_baudrate(dev);
			break;

		case IO_DEVICE_USB:
			break;

		case IO_DEVICE_1394:
			break;

		case IO_DEVICE_PARALLEL:
			break;

		default:
			break;
	}

	return IO_OK;
}

int io_free (io_device *dev)
	/* Frees a device struct */
{
	free(dev);

	return IO_OK;
}

int io_write (io_device *dev, char *bytes, int size)
	/* Called to write "bytes" to the IO device */
{
  int len;
  
  len = write(dev->device_fd, bytes, size);
  if (len != size)
    return IO_ERROR;
  
  /* wait till all bytes are really sent */
#if HAVE_TERMIOS_H
  tcdrain(dev->device_fd);
#else
  ioctl(dev->device_fd, TCDRAIN, 0);
#endif
  return IO_OK;
}

int io_read (io_device *dev, char *bytes, int size) 
	/* Reads data from the device into the "bytes" buffer.
	   "bytes" should be large enough to hold all the data.
	*/
{
  int ret, buflen=0;
  int n;
  fd_set readfds;
  int len;
  struct timeval timeout;
  
  while (buflen < size) 
    {

      /* set parameters for select call */ 
      n = dev->device_fd + 1;
      FD_ZERO(&readfds);
      FD_SET(dev->device_fd, &readfds);
      timeout.tv_sec = 0;
      timeout.tv_usec = 5000000; /* 5 seconds */

      ret = select(n, &readfds, NULL, NULL, &timeout);
      if(ret<=0)
	{ 
	  if (ret<0) 
	    {
	      if (errno==EINTR) continue;
	      perror("select");
	    }
	  else
            fprintf(stderr, "io_recvbuffer: no answer received...\n");
	  return IO_ERROR;
	}
      else
	{
	  if (FD_ISSET(dev->device_fd, &readfds)) 
	    {
	      /* Keep appending in blocks of IO_MAX_BUF_LEN */
	      len = read(dev->device_fd, bytes + buflen,
			 IO_MAX_BUF_LEN - buflen);
	      buflen += len;
	    } 
	  else 
	    return IO_ERROR;
	}
    }
  
  return IO_OK;
}

/*
   Serial port specific helper functions
   ----------------------------------------------------------------
*/

int io_serial_set_baudrate(io_device *dev)
	/* Called to set the baud rate */
{
#if HAVE_TERMIOS_H
  /* termios */
  struct termios tio;
  
  if (tcgetattr(dev->device_fd, &tio) < 0) {
    perror("tcgetattr");
    return IO_ERROR;
  }
  tio.c_iflag = 0;
  tio.c_oflag = 0;
  tio.c_cflag = CS8 | CREAD | CLOCAL;
  tio.c_lflag = ICANON;
  tio.c_cc[VMIN] = 1;
  tio.c_cc[VTIME] = 5;
  cfsetispeed(&tio, io_serial_baudconv(dev->speed));
  cfsetospeed(&tio, io_serial_baudconv(dev->speed));
  if (tcsetattr(dev->device_fd, TCSANOW, &tio) < 0) {
    perror("tcsetattr");
    return IO_ERROR;
  }
# else
  /* sgtty */
  struct sgttyb ttyb;
  
  if (ioctl(dev->device_fd, TIOCGETP, &ttyb) < 0) {
    perror("ioctl(TIOCGETP)");
    return IO_ERROR;
  }
  ttyb.sg_ispeed = dev->speed;
  ttyb.sg_ospeed = dev->speed;
  ttyb.sg_flags = 0;
  if (ioctl(dev->device_fd, TIOCSETP, &ttyb) < 0) {
    perror("ioctl(TIOCSETP)");
    return IO_ERROR;
  }
# endif

  return IO_OK;
}

static speed_t io_serial_baudconv(int baud)
	/* Called to convert a int baud to the POSIX enum value */
{
#define BAUDCASE(x)     case (x): { ret = B##x; break; }
  speed_t ret;
  
  ret = (speed_t) baud;
  switch (baud) {
    /* POSIX defined baudrates */
    BAUDCASE(0);
    BAUDCASE(50);
    BAUDCASE(75);
    BAUDCASE(110);
    BAUDCASE(134);
    BAUDCASE(150);
    BAUDCASE(200);
    BAUDCASE(300);
    BAUDCASE(600);
    BAUDCASE(1200);
    BAUDCASE(1800);
    BAUDCASE(2400);
    BAUDCASE(4800);
    BAUDCASE(9600);
    BAUDCASE(19200);
    BAUDCASE(38400);
    
    /* non POSIX values */
#ifdef B7200
    BAUDCASE(7200);
#endif
#ifdef B14400
    BAUDCASE(14400);
#endif
#ifdef B28800
    BAUDCASE(28800);
#endif
#ifdef B57600
    BAUDCASE(57600);
#endif
#ifdef B115200
    BAUDCASE(115200);
#endif
#ifdef B230400
    BAUDCASE(230400);
#endif
    
  default:
    fprintf(stderr, "baudconv: baudrate %d is undefined; using as is\n", baud);
  }
  
  return ret;
#undef BAUDCASE
}

/*
   USB specific functions
   ----------------------------------------------------------------
*/


/*
   1394 specific functions
   ----------------------------------------------------------------
*/


/*
   Parallel port specific functions
   ----------------------------------------------------------------
*/

