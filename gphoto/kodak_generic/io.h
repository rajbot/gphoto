#ifndef IO_H
#define IO_H
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>

#ifndef TRUE
#define TRUE (0==0)
#endif

#ifndef FALSE
#define FALSE (1==0)
#endif

#if HAVE_TERMIOS_H
# include <termios.h>
#else
#  if HAVE_SYS_IOCTL_H
#   include <sys/ioctl.h>
#  endif
#  include <sgtty.h>
# endif

#define IO_MAX_BUF_LEN 4096

typedef struct io_link {
  int device_fd;
  int buflen;
  char buf[IO_MAX_BUF_LEN];
} io_link;

/* opens device for operation. returns NULL on failure */
io_link *io_open(char *devicename);

/* closes the specified link */
int io_close(io_link *link);

/* sets the baud rate on the specified link */
int io_set_baudrate(io_link *link, int baudrate);

/* write 'nbytes' of buf[] to file descriptor specified by link */
int io_sendbuffer(io_link *link, char *buf, int nbytes);

/* read data from file descriptor till 'nbytes' Bytes are in the buffer */
int io_recvbuffer(io_link *link, int nbytes);

#endif














