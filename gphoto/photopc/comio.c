#ifndef LINT
static char *rcsid="$Id$";
#endif

/*
	$Log$
	Revision 1.2  2000/08/24 05:04:27  scottf
	adding language support

	Revision 1.1.1.1.2.1  2000/07/05 11:07:49  ole
	Preliminary support for the Olympus C3030-Zoom USB by
	Fabrice Bellet <Fabrice.Bellet@creatis.insa-lyon.fr>.
	(http://lists.styx.net/archives/public/gphoto-devel/2000-July/003858.html)
	
	Revision 1.3  1998/10/18 13:18:27  crosser
	Put RCS logs and I.D. into the source

	Revision 1.2  1998/08/01 13:12:41  lightner
	Four com ports support
	
	Revision 1.1  1998/01/18 02:16:45  crosser
	Initial revision
	
*/

#include <stdio.h>
#include <dos.h>

#define MDMDAT1 0x03F8            /* Address of modem port 1 data */
#define MDMCOM1 0x03FB            /* Address of modem port 1 command */
#define MDMSTS1 0x03FD            /* Address of modem port 1 status  */

#define MDMDAT2 0x02F8            /* Address of modem port 2 data */
#define MDMCOM2 0x02FB            /* Address of modem port 2 command */
#define MDMSTS2 0x02FD            /* Address of modem port 2 status */

#define MDMDAT3 0x03E8            /* Address of modem port 3 data */
#define MDMCOM3 0x03EB            /* Address of modem port 3 command */
#define MDMSTS3 0x03ED            /* Address of modem port 3 status  */

#define MDMDAT4 0x02E8            /* Address of modem port 4 data */
#define MDMCOM4 0x02EB            /* Address of modem port 4 command */
#define MDMSTS4 0x02ED            /* Address of modem port 4 status  */

#define MDMINTV 0x000C            /* Com 1 interrupt vector */
#define MDINTV2 0x000B            /* Com 2 interrupt vector */
#define MDMINTO 0x0EF             /* Mask to enable IRQ3 for port 1 */
#define MDINTO2 0x0F7             /* Mask to enable IRQ4 for port 2 */
#define MDMINTC 0x010             /* Mask to Disable IRQ4 for port 1 */
#define MDINTC2 0x008             /* Mask to Disable IRQ3 for port 2 */
#define INTCONT 0x0021            /* 8259 interrupt controller ICW2-3 */
#define INTCON1 0x0020            /* Address of 8259 ICW1 */

#define COM_BUFF_SIZE 2048        /* Communications port buffer size */
#define XOFFPT  COM_BUFF_SIZE*3/4 /* chars in buff before sending XOFF */
#define XONPT   COM_BUFF_SIZE*1/4 /* chars in buff to send XON after XOFF */
#define XOFF    0x13              /* XOFF value */
#define XON     0x11              /* XON value */

/*****************************************************************************/
/* function prototypes                                                       */

void TTinit(int port, long speed);/* Initialize the communications system */
int ttopen();                     /* Open a port for communications */
int ttclose( void );              /* Close the communications port */
int ttchk( void );                /* Return count of received characters */
void ttoc( unsigned char );       /* Output a character to the com port */
int ttinc( void );                /* Input a character from circular buffer */
void ttflui( void );              /* Flush circular buffer of characters */
int dobaud( long );               /* Set the baud rate for the port */
void coms( int );                 /* Establish modem data */
void serini( void );              /* Initialize the com port for interrupts */
void serrst( void );              /* Reset the com port to original settings */
void interrupt serint( void );    /* Com port receiver ISR */


/*****************************************************************************/
/* Global Data                                                               */

unsigned int port;                /* COM port */
long speed;                       /* BAUD rate */
char parity[5];                   /* Parity setting */
unsigned int databits;            /* Number of Data bits */
unsigned int stopbits;            /* Number of Stop bits */

/*****************************************************************************/
/* External variables                                                        */



/*****************************************************************************/
/* Local Static Data                                                         */

static char buffer[COM_BUFF_SIZE];/* Circular buffer */
static char *inptr;               /* Pointer to input point of circular buff*/
static char *outptr;              /* Pointer to output point of circular buff*/
static int  count = 0;            /* Number of characters in buffer */

struct mdminfo {                  /* struct to hold current com port info */
    unsigned int mddat;             /* 8250 data register */
    unsigned int mdstat;            /* 8250 line-status register */
    unsigned int mdcom;             /* 8250 line-control register */
    unsigned char mden;             /* 8259 IRQ enable mask */
    unsigned char mddis;            /* 8259 IRQ disable mask */
    unsigned char mdintv;           /* Interrupt for selected com port */
} modem ;

void interrupt (*oldvec)();       /* Vector of previous com interrupt */
int portin = 0;                   /* Flag to indicate com port is open */
int xofsnt = 0;                   /* Flag to indicate an XOFF transmitted */
int xofrcv = 0;                   /* Flag to indicate an XOFF received */

/*****************************************************************************/

/*  T T I N I T  -- Initialize the communications system */

void TTinit(int new_port, long new_speed) {
    port = new_port;
    speed = new_speed;
    strcpy(parity,"NONE");
    databits = 8;
    stopbits = 0;
}


/*  T T O P E N  -- Open the communications port */

ttopen() {
    if (portin == 0) {            /* Ignore call if already open */
        switch (port) {
            case 1:
                coms(1);             /* COM 1 */
                break;
            case 2:
                coms(2);             /* COM 2 */
                break;
            case 3:
                coms(3);             /* COM 3 */
                break;
            case 4:
                coms(4);             /* COM 4 */
                break;
            default:                 /* others not supported, return error */
                return(-1);
        }
        dobaud(speed);               /* Set baud rate */
        serini();                    /* enable interrupt handler */
    }
    return(0);                    /* return success */
}


/*  T T C L O S E --  Close the communications port  */

ttclose() {
    if (portin != 0)              /* Ignore if port is already closed */
        serrst();                    /* otherwise disable interrupts */
    return(0);                    /* return success */
}




/* T T C H K  --  Return a count of characters at the serial port */

ttchk() {
    return( count );              /* return maintained count */
}


/* T T O C -- Output a character to the current serial port */

void ttoc( unsigned char c ) {

    while( (inportb(modem.mdstat) & 0x20) == 0 )
       ;                          /* Wait til transmitter is ready */
    outportb(modem.mddat,c);      /* then output the character */
}


/* T T F L U I  --  Clear the input buffer of characters */


void ttflui() {

#ifdef XONXOFF
    if (xofsnt){                  /* Check if XON should be sent after XOFF */
       xofsnt = 0;                  /* if so then reset XOFF sent status */
       ttoc(XON);                   /* and send the XON */
       }
#endif
    disable();                    /* NO interrupts allowed now */
    inptr = outptr = buffer;      /* Reset input out output pointers */
    count = 0;                    /* Set received characters count to 0 */
    enable();                     /* Now interrupts are ok */
}


/* T T I N C  -- Read a character from serial ports circular buffer */

ttinc() {
    int c;
    register char * ptr;

#ifdef XONXOFF
    if (count < XONPT && xofsnt){ /* Check if XON should be sent after XOFF */
       xofsnt = 0;                  /* if so then reset XOFF sent status */
       ttoc(XON);                   /* and send the XON */
       }
#endif

    while (count == 0)            /* If no characters have arrived then */
        ;                            /* wait til one arrives */

    ptr = outptr;                 /* Save address of buffer output point */

    c = *ptr++;                   /* Get this character and increment ptr */

                                  /* See if circular buff should be wrapped */
    if (ptr == &buffer[COM_BUFF_SIZE])
        ptr = buffer;                /* if so then save new output point */

    disable();                    /* NO interrupts allowed now */
    outptr = ptr;                 /* Save the address of output point */
    count--;                      /* Decrement count of received characters */
    enable();                     /* Interrupts can continue now */

    return(c);                    /* Return the received character */
}


/* D O B A U D  --  Set the baud rate for the current port */

dobaud( long baudrate ) {
   unsigned char portval;
   unsigned char blo, bhi;
   switch (baudrate) {            /* Get 8250 baud rate divisor values */
       case 50:     bhi = 0x9;  blo = 0x00;  break;
       case 75:     bhi = 0x6;  blo = 0x00;  break;
       case 110:    bhi = 0x4;  blo = 0x17;  break;
       case 150:    bhi = 0x3;  blo = 0x00;  break;
       case 300:    bhi = 0x1;  blo = 0x80;  break;
       case 600:    bhi = 0x0;  blo = 0xC0;  break;
       case 1200:   bhi = 0x0;  blo = 0x60;  break;
       case 1800:   bhi = 0x0;  blo = 0x40;  break;
       case 2000:   bhi = 0x0;  blo = 0x3A;  break;
       case 2400:   bhi = 0x0;  blo = 0x30;  break;
       case 4800:   bhi = 0x0;  blo = 0x18;  break;
       case 9600:   bhi = 0x0;  blo = 0x0C;  break;
       case 19200:  bhi = 0x0;  blo = 0x06;  break;
       case 38400:  bhi = 0x0;  blo = 0x03;  break;
       case 57600:  bhi = 0x0;  blo = 0x02;  break;
       case 115200: bhi = 0x0;  blo = 0x01;  break;

       default:                   /* Return failure if baud unsupported */
           return(-1);
   }

   portval = inportb(modem.mdcom);/* Save current value of command register */

                                  /* In order to set the baud rate the */
                                  /* high bit of command data register is */
   outportb(modem.mdcom,portval | 0x80 ); /* set before sending baud data */

   outportb(modem.mddat,blo);     /* Set LSB Baud-Rate divisor for baud */
   outportb(modem.mddat + 1,bhi); /* Set MSB Baud-Rate divisor for baud */

   outportb(modem.mdcom,portval); /* Reset original command register value */

   return(0);                     /* Return success */
}


/*  C O M S  --  Set up the modem structure for the specified com port */

void coms( int portid ) {

    if (portid == 1) {            /* Port data for COM 1 */
        modem.mddat = MDMDAT1;       /* Port 1 Data register */
        modem.mdstat = MDMSTS1;      /* Port 1 Status register */
        modem.mdcom = MDMCOM1;       /* Port 1 Command register */
        modem.mddis = MDMINTC;       /* Port 1 8259 IRQ4 disable mask */
        modem.mden = MDMINTO;        /* Port 1 8259 IRQ4 enable mask */
        modem.mdintv = MDMINTV;      /* Port 1 interrupt number */
    }
    else if (portid == 2) {       /* Port data for COM 2 */
        modem.mddat = MDMDAT2;       /* Port 2 Data register */
        modem.mdstat = MDMSTS2;      /* Port 2 Status register */
        modem.mdcom = MDMCOM2;       /* Port 2 Command register */
        modem.mddis = MDINTC2;       /* Port 2 8259 IRQ4 disable mask */
        modem.mden = MDINTO2;        /* Port 2 8259 IRQ4 enable mask */
        modem.mdintv = MDINTV2;      /* Port 2 interrupt number */
    }
    else if (portid == 3) {            /* Port data for COM 3 */
        modem.mddat = MDMDAT3;       /* Port 3 Data register */
        modem.mdstat = MDMSTS3;      /* Port 3 Status register */
        modem.mdcom = MDMCOM3;       /* Port 3 Command register */
        modem.mddis = MDMINTC;       /* Port 3 8259 IRQ4 disable mask */
        modem.mden = MDMINTO;        /* Port 3 8259 IRQ4 enable mask */
        modem.mdintv = MDMINTV;      /* Port 3 interrupt number */
    }
    else if (portid == 4) {       /* Port data for COM 4 */
        modem.mddat = MDMDAT4;       /* Port 4 Data register */
        modem.mdstat = MDMSTS4;      /* Port 4 Status register */
        modem.mdcom = MDMCOM4;       /* Port 4 Command register */
        modem.mddis = MDINTC2;       /* Port 4 8259 IRQ4 disable mask */
        modem.mden = MDINTO2;        /* Port 4 8259 IRQ4 enable mask */
        modem.mdintv = MDINTV2;      /* Port 4 interrupt number */
    }
}

/* S E R I N I  -- initialize the serial port for interrupts */

void serini() {
    unsigned char portval;

    if (portin == 0) {            /* Ignore if already open */
        portin = 1;                  /* save port open status */
        inptr = outptr = buffer;     /* set circular buffer pointers */
        count = 0;                   /* indicate no characters received */
        oldvec=getvect(modem.mdintv);/* save old com interrupt */
        setvect(modem.mdintv,serint);/* set SERINT as communications ISR */

        portval = 0;              /* Byte value to output to the Line */
                                  /* Control Register (LCR) to set the */
                                  /* Parity, Stopbits, Databits */
                                  /* Start out with all bits zero */

        if (strcmp(parity,"EVEN") == 0)
           portval |= 0x8;        /* Set bit 3 on for odd parity */
        else if (strcmp(parity,"ODD") == 0)
           portval |= 0x18;       /* Set bits 3 and 4 on for even parity */
                                  /* Leave bits 3 and 4 off for no parity */


        if (stopbits == 2)        /* Set bit 2 on if 2 Stopbits are used */
           portval |= 0x4;
                                  /* Leave bit 2 off for 1 Stopbit */

        if (databits == 6)        /* Set bit 0 on for 6 data bits */
           portval |= 0x1;
        else if (databits == 7)   /* Set bit 1 on for 7 data bits */
           portval |= 0x2;
        else if (databits == 8)   /* Set bits 0 and 1 on for 8 data bits */
           portval |= 0x3;
                                  /* Leave bits 0 and 1 off for 5 data bits */

        outportb(modem.mdcom,portval);  /* Output the settings to the LCR */


        outportb(modem.mdcom + 1,0xb); /* Assert OUT2, RTS, DTR */

        inportb(modem.mddat);        /* Clear any left over characters */
        outportb(modem.mddat+1,0x1); /* Enable receiver interrupts */

        portval = inportb(INTCONT);  /* Read 8259 interrupt enable mask */
        outportb(INTCONT,modem.mden & portval); /*Set bit on for com IRQ */
    }
}


/* S E R R S T -- Reset serial port interrupts */

void serrst() {
    unsigned char portval;

    if (portin != 0) {            /* Ignore if interrupts already disabled */
        portin = 0;                    /* save port closed status */
        portval = inportb(INTCONT);    /* Read 8259 interrupt enable mask */
        outportb(INTCONT,modem.mddis | portval);/*Set bit off for com IRQ */
        setvect(modem.mdintv,oldvec);  /* return original interrupt vector */
    }
}


/*  S E R I N T  -- Serial interrupt handler, recieves incoming characters */

void interrupt serint() {

more:
    *inptr++=inportb(modem.mddat);/* Quickly read arriving character */
    count++;                      /* Increment received count */

#ifdef XONXOFF
    if (count > XOFFPT && xofsnt != 1){ /* If buffer almost full then */
       ttoc(XOFF);                /* send an XOFF */
       xofsnt = 1;                /* and save XOFF sent status */
       }
#endif
    disable();                    /* NO interrupts are allowed while */
                                  /* new input pointer is stored */
    if (inptr == &buffer[COM_BUFF_SIZE]) /* At end of circular buff? */
       inptr = buffer;                /* if so then save new output point */

    enable();                     /* Interrupts ok now */

#define LAST_LOOK
#ifdef LAST_LOOK
    if (inportb(modem.mddat + 5) & 1) /* another character ready? */
	goto more;
#endif

    outportb(0x20,0x20);          /* send End Of Interrupt to 8259 */
}

