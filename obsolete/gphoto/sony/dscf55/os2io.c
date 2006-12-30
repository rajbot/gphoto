#define INCL_OS2
#define INCL_WIN
#define INCL_DOSDEVICES
#define INCL_DOSFILEMGR    /* File System values */
#define INCL_DOSDEVIOCTL   /* DosDevIOCtl values */
#define INCL_DOSFILEMGR   /* File Manager values */
#include <os2.h>
#include <stdio.h>
#include <i86.h>

#define	usleep(a)	delay(((a)/1000))


HFILE   hf;                /* File handle for the device           */
ULONG   usBPS = 9600;      /* Bit rate to set the COM port to      */
ULONG   ulParmLen = 2;     /* Maximum size of the parameter packet */
ULONG   ulAction;          /* Action taken by DosOpen              */
APIRET  ulrc;              /* Return code                          */

HFILE	dscf55_fd;
 
typedef struct tagExtendedBaud
{
    ULONG	rate;
    BYTE	frac;
} ExtendedRate;

typedef struct tagDCBInfo
{
    USHORT	time_1;
    USHORT	time_2;
    BYTE	b1;
    BYTE	b2;
    BYTE	b3;
    BYTE	e;
    BYTE	b;
    BYTE	xon;
    BYTE	xoff;
} DCBInfo ;

DCBInfo		dsi;
DCBInfo		dst;



/***************************************************************
*
*
*/
int TransferRateID(int baud)
{
	int r = 0;

	switch (baud)
	{
		case 115200: 
			r = 4;
			break;
		case 57600: 
			r = 3;
			break;
		case 38400: 
			r = 2;
			break;
		case 19200: 
			r = 1;
			break;	/* works on sun */
		default:
		case 9600:
			r = 0;
			break;	/* works on sun */
	}

	return r;
}


/***************************************************************
*
*
*/
int SetPortSpeed()
{
    	ExtendedRate er;

/*
	printf("Setting speed to %ld", usBPS);
*/

	if(usBPS>19200)
	{
	    er.rate = usBPS;
	    er.frac = 0;
    
	    ulrc = DosDevIOCtl(dscf55_fd,                /* Device handle                  */
			    IOCTL_ASYNC,       /* Serial-device control          */
			    0x43, /* Sets extended bit rate                  */
			    (PULONG) &er,   /* Points at bit rate             */
			    sizeof(ExtendedRate),     /* Maximum size of parameter list */
			    &ulParmLen,        /* Size of parameter packet       */
			    NULL,              /* No data packet                 */
			    0,                 /* Maximum size of data packet    */
			    NULL);             /* Size of data packet            */
	}
	else
	{
	    USHORT rate = usBPS;
    
	    ulrc = DosDevIOCtl(dscf55_fd,                /* Device handle                  */
			    IOCTL_ASYNC,       /* Serial-device control          */
			    ASYNC_SETBAUDRATE, /* Sets bit rate                  */
			    (PULONG) &rate,   /* Points at bit rate             */
			    sizeof(rate),     /* Maximum size of parameter list */
			    &ulParmLen,        /* Size of parameter packet       */
			    NULL,              /* No data packet                 */
			    0,                 /* Maximum size of data packet    */
			    NULL);             /* Size of data packet            */
	}

/*
	printf("Set Port Speed returned %ld\n", ulrc);
*/

	delay(500);

	return 0;
}


/***************************************************************
**
**
*/
int ConfigDSCF55Speed(char *spd_buf, int verbose)
{
  ULONG usBPS = 9600;

	if(!strcmp(spd_buf, "115200"))
		usBPS = 115200;
#if defined(B76800)
	else if(!strcmp(spd_buf,  "76800"))
		usBPS = 76800;
	else
#endif
	if(!strcmp(spd_buf,  "57600"))
		usBPS = 57600;
	else if(!strcmp(spd_buf,  "38400"))
		usBPS = 38400;
	else if(!strcmp(spd_buf,  "19200"))
		usBPS = 19200;
	else if(!strcmp(spd_buf,   "9600"))
		usBPS = 9600;

	if(verbose > 1)
		printf("Speed set to %u (%s bps)\n", usBPS, spd_buf);

  return usBPS;
}

/***************************************************************
*
*
*/
void ClosePort()
{
     ulrc = DosClose(dscf55_fd);

}
 
 

/***************************************************************
*
*
*/
typedef struct _tagbaudtable
{
    ULONG	bitrate;
    BYTE	fraction;
    ULONG	minimum;
    BYTE	minfrac;
    ULONG	maximum;
    BYTE	maxfrac;
} baudtable;

baudtable	bt;
    


int InitSonyDSCF55(char *devicename)
{
	char	buffer[256];
	ULONG	len=sizeof(baudtable);
	BYTE	info[4];
	ULONG	ulParmLen = 4;

	ulrc = DosOpen(devicename, &dscf55_fd, &ulAction, 0, FILE_NORMAL,
			FILE_OPEN,
		       0x12,
                    (PEAOP2) NULL);

	printf("DosOpen returned %d\n", dscf55_fd);
 
    ulParmLen = sizeof(DCBInfo);

    ulrc = DosDevIOCtl(dscf55_fd,                /* Device handle                  */
	   IOCTL_ASYNC,       /* Serial-device control          */
	   0x73, /* Sets bit rate                  */
	   NULL,   /* Points at bit rate             */
	   NULL,     /* Maximum size of parameter list */
	   &ulParmLen,        /* Size of parameter packet       */
	   &dsi,              /* No data packet                 */
	   ulParmLen,                 /* Maximum size of data packet    */
	   &ulParmLen);             /* Size of data packet            */

	if(ulrc!=0)
	    printf("GetDCBInfo returned %d\n", ulrc);
	else
	{
	    printf("%x %x \n", dsi.time_1, dsi.time_2);
	    printf("%x %x %x\n", dsi.b1, dsi.b2, dsi.b3);
	    printf("%x %x %x %x\n", (USHORT)dsi.e, (USHORT)dsi.b,(USHORT) dsi.xon,(USHORT) dsi.xoff);
	}

	memcpy(dst,dsi,sizeof(DCBInfo));

	dsi.b1=0;
	dsi.b2=0;
	dsi.b3 |=5;
	dsi.time_2 = 60;
	dsi.time_1 = 60;

    ulParmLen = sizeof(DCBInfo);

    ulrc = DosDevIOCtl(dscf55_fd,                /* Device handle                  */
	   IOCTL_ASYNC,       /* Serial-device control          */
	   0x53, /* Sets bit rate                  */
	   &dsi,   /* Points at bit rate             */
	   ulParmLen,     /* Maximum size of parameter list */
	   &ulParmLen,        /* Size of parameter packet       */
	   NULL,              /* No data packet                 */
	   0,                 /* Maximum size of data packet    */
	   NULL);             /* Size of data packet            */

	if(ulrc!=0)
	    printf("SetDCBInfo returned %d\n", ulrc);

	delay(200);

	ulrc = SetPortSpeed();

	printf("DosOpen returned %ld\n", ulrc);

	return TRUE;
}


/***************************************************************
*
*
*/
void CloseSonyDSCF55()
{
    ClosePort();
}


/***************************************************************
*
*
*/
int dscSetSpeed(int speed)
{
	usleep(50000);

	switch(speed)
	{
		case 115200:
    		case 57600:
		case 38400:
		case 19200:
			usBPS=speed;
			break;
		default:
			usBPS=9600;
			break;
	}

	return SetPortSpeed();
}


/***************************************************************
*
*
*/
int extRead(unsigned char *buffer, int *length)
{
	APIRET   rc;              /* Return code */
	ULONG    BytesRead;       /* Bytes read (returned) */
	ULONG    Bytes = 3;
	ULONG	ulParmLen = 4;
	USHORT	info[2];
	BYTE	*p = buffer;
	USHORT	ByteCount=0;
	int 	bcount=1;

	delay(5);

	do
	{
		char Byte;
		rc = DosRead(dscf55_fd, &Byte, bcount, &BytesRead);


		if(BytesRead)
		{
			*(p++)=Byte;
			ByteCount++;

			if((ByteCount==*length) || Byte==0xc1)
				break;
		}

	}while(BytesRead==1);

	if(rc != 0)
	{
		printf("DosRead error: return code = %ld", rc);
		return 0;
	}

	*length=ByteCount;
 
	return ByteCount;
}


/***************************************************************
*
*
*/
int ReadCommByte(unsigned char *byte)
{
	static unsigned char buf[256];
	static int bytes_read = 0;
	static int bytes_returned = 0;
	int	buflen=256;

	if(bytes_returned < bytes_read)
	{
		*byte = buf[bytes_returned++];
		return 1;
	}

	bytes_read = extRead(buf, &buflen);

	bytes_returned = 0;

	if(bytes_read)
		*byte = buf[bytes_returned++];

	return (bytes_read > 0) ? 1 : bytes_read;
}


/***************************************************************
*
*
*/
int extWrite(unsigned char *buffer, int length)
{
    	APIRET	rc;
	ULONG bytecount;


	DosWrite(dscf55_fd, buffer, (ULONG)length, &bytecount);

	delay(1);

	return bytecount;
}


/***************************************************************
*
*
*/
void DumpData(unsigned char *buffer, int length)
{
	int n=0;

	printf("Dumping :");

	for(n=0; n<length; n++)
	{
		printf("%u ", (int) ((unsigned char )buffer[n]));
	}

	fflush(stdout);
}


