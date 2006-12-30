/*
 *
 *   (c) Copyright 1999, Beat Christen <spiff@longstreet.ch>
 *
 */

#include "io.h"
/****************************************************************************/
/* local functions */
static speed_t baudconv(int rate);
/* end of local function declarations */


/****************************************************************************/
/* opens device for operation */

io_link *io_open(char *devicename)
{ 
  io_link *link;

  link = (io_link *) malloc(sizeof(io_link));
  if(link==NULL) return NULL;

  /* open the device specified by devicename */
  link->device_fd = open(devicename, O_RDWR|O_NDELAY);
  if(link->device_fd==-1) 
    {
      fprintf(stderr, "io_open: failed to open ");
      perror(devicename);
      free(link);
      return NULL;
    }

  link->buflen=0;

  return link;
}


/****************************************************************************/
/* closes specified device */
int io_close(io_link *link)
{

  if(!link) return FALSE;

  if(close(link->device_fd)==-1)
    perror("io_close() tried closing device file descriptor");
  
  free(link);

  return TRUE;
}


/****************************************************************************/
/* sets the baud rate on the specified link */

int io_set_baudrate(io_link *link, int baudrate)
{
#if HAVE_TERMIOS_H
  /* termios */
  struct termios tio;
  
  if (tcgetattr(link->device_fd, &tio) < 0) {
    perror("tcgetattr");
    return FALSE;
  }
  tio.c_iflag = 0;
  tio.c_oflag = 0;
  tio.c_cflag = CS8 | CREAD | CLOCAL;
  tio.c_lflag = 0;
  tio.c_cc[VMIN] = 1;
  tio.c_cc[VTIME] = 5;
  cfsetispeed(&tio, baudconv(baudrate));
  cfsetospeed(&tio, baudconv(baudrate));
  if (tcsetattr(link->device_fd, TCSANOW, &tio) < 0) {
    perror("tcsetattr");
    return FALSE;
  }
# else
  /* sgtty */
  struct sgttyb ttyb;
  
  if (ioctl(link->device_fd, TIOCGETP, &ttyb) < 0) {
    perror("ioctl(TIOCGETP)");
    return FALSE;
  }
  ttyb.sg_ispeed = baudrate;
  ttyb.sg_ospeed = baudrate;
  ttyb.sg_flags = 0;
  if (ioctl(link->device_fd, TIOCSETP, &ttyb) < 0) {
    perror("ioctl(TIOCSETP)");
    return FALSE;
  }
# endif

  return TRUE;
}

static speed_t
baudconv(int baud)
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


/****************************************************************************/
/* send bytes to the camera */

int io_sendbuffer(io_link *link, char *buf, int nbytes)
{
  int len;
  
  len = write(link->device_fd, buf, nbytes);
  if (len != nbytes)
    return FALSE;
  
  /* wait till all bytes are really sent */
#if HAVE_TERMIOS_H
  tcdrain(link->device_fd);
#else
  ioctl(link->device_fd, TCDRAIN, 0);
#endif
  return TRUE;
}


/****************************************************************************/
/* read data from file descriptor till at least 'nbytes' Bytes are in
 * the link-buffer 
 */
int io_recvbuffer(io_link *link, int nbytes) 
{
  int ret;
  int n;
  fd_set readfds;
  int len;
  struct timeval timeout;
  
  while (link->buflen < nbytes) 
    {

      /* set parameters for select call */ 
      n = link->device_fd + 1;
      FD_ZERO(&readfds);
      FD_SET(link->device_fd, &readfds);
      timeout.tv_sec = 0;
      timeout.tv_usec = 500000; /* 500ms */

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
	  return FALSE;
	}
      else
	{
	  if (FD_ISSET(link->device_fd, &readfds)) 
	    {
	      len = read(link->device_fd, link->buf + link->buflen,
			 IO_MAX_BUF_LEN - link->buflen);
	      link->buflen += len;
	    } 
	  else 
	    return FALSE;
	}
    }
  
  return TRUE;
}

