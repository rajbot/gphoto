#ifndef _RS232_H
#define _RS232_H


/* Prevent Overruns ?? */
#define MDC800_DEFAULT_COMMAND_DELAY 50000
#define MDC800_RESEND_COMMAND_DELAY	 500000  	

/* Long Timeout for Functions that needs time (Take Photo, delete..) */
#define MDC800_LONG_TIMEOUT			5

/* 20sec Timeout for Flashlight */
#define MDC800_TAKE_PICTURE_TIMEOUT 20	

int mdc800_rs232_sendCommand (char* , char * , int );
int mdc800_rs232_waitForCommit (char commandid);
int mdc800_rs232_receive (char * , int );
int mdc800_rs232_download (char *, int);

#endif
