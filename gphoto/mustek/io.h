#ifndef _IO_H
#define _IO_H

int mdc800_io_openDevice (char*,int);
int mdc800_io_closeDevice ();
int mdc800_io_sendCommand (char ,char,char,char,char *,int );
int mdc800_io_sendUSBCommand (char, char, char, char, char, char, char, char*, int);
int mdc800_io_changespeed (int );

#ifdef _IO_C
int mdc800_device_handle=-1;		// Device handle
#else
extern int mdc800_device_handle;
#endif

#endif
