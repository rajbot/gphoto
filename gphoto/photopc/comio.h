/*
	$Id$
*/

/*
	$Log$
	Revision 1.2  2000/08/24 05:04:27  scottf
	adding language support

	Revision 1.1.1.1.2.1  2000/07/05 11:07:49  ole
	Preliminary support for the Olympus C3030-Zoom USB by
	Fabrice Bellet <Fabrice.Bellet@creatis.insa-lyon.fr>.
	(http://lists.styx.net/archives/public/gphoto-devel/2000-July/003858.html)
	
	Revision 1.2  1998/10/18 13:18:27  crosser
	Put RCS logs and I.D. into the source

	Revision 1.1  1998/01/18 02:16:45  crosser
	Initial revision
	
*/

#ifndef _COMIO_H
#define _COMIO_H

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

#endif
