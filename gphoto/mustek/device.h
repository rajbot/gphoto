#ifndef _DEVICE_H
#define _DEVICE_H

int	mdc800_device_changespeed (int, int);
int	mdc800_device_open (char *,int);
int	mdc800_device_read (int, char*, int,int );
void	mdc800_device_flush (int);

int 	mdc800_device_write (int, char*, int);
int 	mdc800_device_close (int );
int 	mdc800_device_probeUSB (int );
int 	mdc800_device_USB_detected ();

// Timeout (sec, usec)
#define MDC800_TTY_TIMEOUT_SEC  0
#define MDC800_TTY_TIMEOUT_USEC 250000

#endif
