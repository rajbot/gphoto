/******************************************************************************/
/* See lowlevel.h for licence and details.			              */
/******************************************************************************/


/*************************/
/* Included header files */
/*************************/
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <gphoto2.h>
#include "library.h"
#include "lowlevel.h"


/***************/
/* Definitions */
/***************/
#define DEFAULT_TIMEOUT	500
#define STX	0x02	/****************/
#define ETX	0x03	/*		*/
#define ENQ	0x05	/*		*/
#define ACK	0x06	/*		*/  
#define XOFF	0x11	/* ESC quotes.	*/     
#define XON	0x13	/*		*/ 
#define NACK	0x15	/*		*/     
#define ETB	0x17	/*		*/
#define ESC	0x1b	/****************/
#define EOT	0x04       


/**************/
/* Prototypes */
/**************/
gint l_esc_read (gp_port* device, guchar* c);


gint l_send (gp_port* device, guchar* send_buffer, guint send_buffer_size);


gint l_receive (gp_port* device, guchar** rb, guint* rbs, guint timeout);


/*************/
/* Functions */
/*************/
gint 
l_init (gp_port* device)
{
	guchar 	c;
	gint 	i;
	gint	result;

	g_return_val_if_fail (device, GP_ERROR_BAD_PARAMETERS);

	gp_port_timeout_set (device, DEFAULT_TIMEOUT);
	if ((result = gp_port_open (device)) != GP_OK) return (result);
	for (i = 0; ; i++) {
		/****************/
		/* Write ENQ. 	*/
		/****************/
		if ((result = gp_port_write (device, "\5", 1)) != GP_OK) return (result);
		if ((result = gp_port_read (device, &c, 1)) < 1) {
			/******************************************************/
			/* We didn't receive anything. We'll try up to five   */
			/* time.                                              */
			/******************************************************/
			if (i == 4) {
			        gp_port_close (device);
				return (result);
			}
			continue;
		}
		switch (c) {
		case ACK:
			/******************************************************/
			/* ACK received. We can proceed.                      */
			/******************************************************/
			return (GP_OK);
		default:
			/******************************************************/
			/* The camera seems to be sending something. We'll    */
			/* dump all bytes we get and try again.               */
			/******************************************************/
			while (gp_port_read (device, &c, 1) > 0);
			i--;
			break;
		}
	}
}


gint 
l_exit (gp_port* device)
{
	g_return_val_if_fail (device, GP_ERROR_BAD_PARAMETERS);
	
	return (gp_port_close (device));
}


gint 
l_esc_read (gp_port* device, guchar* c)
{
	gint result;
	
	g_return_val_if_fail (device, GP_ERROR_BAD_PARAMETERS);

	if ((result = gp_port_read (device, c, 1)) < 1) return (result);
	/**********************************************************************/
	/* STX, ETX, ENQ, ACK, XOFF, XON, NACK, and ETB have to be masked by  */
	/* ESC. If we receive one of those (except ETX and ETB) without mask, */
	/* we will not report an error, as it will be recovered automatically */
	/* later. If we receive ETX or ETB, we reached the end of the packet  */
	/* and report a transmission error, so that the error can be          */
	/* recovered.                                                         */
	/* If the camera sends us ESC (the mask), we will not count this byte */
	/* and read a second one. This will be reverted and processed. It     */
	/* then must be one of STX, ETX, ENQ, ACK, XOFF, XON, NACK, ETB, or   */
	/* ESC. As before, if it is not one of those, we'll not report an     */
	/* error, as it will be recovered automatically later.                */
	/**********************************************************************/
	if ((*c == STX ) || (*c == ETX) || (*c == ENQ ) || (*c == ACK) || (*c == XOFF) || (*c == XON) || (*c == NACK) || (*c == ETB)) {
		gp_debug_printf (GP_DEBUG_HIGH, "konica", "Wrong ESC masking!");
		if ((*c == ETX) || (*c == ETB)) return (GP_ERROR_CORRUPTED_DATA);

	} else if (*c == ESC) {
		if ((result = gp_port_read (device, c, 1)) < 1) return (result);
		*c = (~*c & 0xff);
		if ((*c != STX ) && (*c != ETX ) && (*c != ENQ) && (*c != ACK ) && (*c != XOFF) && (*c != XON) && (*c != NACK) && (*c != ETB ) && (*c != ESC))
			gp_debug_printf (GP_DEBUG_HIGH, "konica", "Wrong ESC masking!");
	}
	return (GP_OK);
}


gint 
l_send (gp_port* device, guchar* send_buffer, guint send_buffer_size)
{
	guchar 	c;
	guchar 	checksum;
	/**********************************************************************/
	/*  sb: A pointer to the buffer that we will send.                    */
	/* sbs: Its size.		                                      */
	/**********************************************************************/
	guchar*	sb; 	
	guint	sbs;
	gint 	i;
	gint 	result;

	g_return_val_if_fail (device, GP_ERROR_BAD_PARAMETERS);

	i = 0;
	for (;;) {
		/****************/
		/* Write ENQ.	*/
		/****************/
		if ((result = gp_port_write (device, "\5", 1)) != GP_OK) return (result);
		/****************/
		/* Read.	*/
		/****************/
		if ((result = gp_port_read (device, &c, 1)) < 1) {
			/************************************/
			/* Let's try for a couple of times. */
			/************************************/
			i++;
			if (i == 5) return (result);
			continue;
		}
		switch (c) {
		case ACK:
			/****************************************/
			/* ACK received. We can proceed.	*/
			/****************************************/
			break;
		case NACK:
			/****************************************/
			/* NACK received. We'll start from the  */
			/* beginning.				*/
			/****************************************/
			continue;
		case ENQ:
			/******************************************************/
			/* ENQ received. It seems that the camera would like  */
			/* to send us data, but we do not want it and         */
			/* therefore simply reject it. The camera will try    */
			/* two more times with ENQ to get us to receive data  */
			/* before finally giving up and sending us ACK.       */
			/******************************************************/
			/****************/
			/* Write NACK.	*/
			/****************/
			c = NACK;
			if ((result = gp_port_write (device, &c, 1)) != GP_OK) return (result);
			for (;;) {
				if ((result = gp_port_read (device, &c, 1)) < 1) return (result);
				switch (c) {
				case ENQ: 
					/**************************************/
					/* The camera has not yet given up.   */
					/**************************************/
					continue;
				case ACK:
					/**************************************/
					/* ACK received. We can proceed.      */
					/**************************************/
					break;
				default:
					/**************************************/
					/* This should not happen.            */
					/**************************************/
					return (GP_ERROR_CORRUPTED_DATA);
				}
				break;
			}
			break;
		default:
			/******************************************************/
			/* The camera seems to send us data. We'll            */
			/* simply dump it and try again.                      */
			/******************************************************/
			continue;
		}
		break;
	}
	/********************************************************/
	/* We will write:			 		*/
	/*  - STX						*/
	/*  - low order byte of (send_buffer_size + 5)		*/
	/*  - high order byte of (send_buffer_size + 5)		*/
	/*  - 'send_buffer_size' bytes data plus ESC quotes	*/
	/*  - ETX						*/
	/*  - 1 byte for checksum plus ESC quotes		*/
	/*							*/
	/* The checksum covers all bytes after STX and before	*/
	/* the checksum byte(s).				*/
	/********************************************************/
	sbs = send_buffer_size + 5;
	sb = g_new (guchar, sbs);
	sb[0] = STX;
	sb[1] = send_buffer_size;
	sb[2] = send_buffer_size >> 8;
	checksum  = sb[1];
	checksum += sb[2];
	for (i = 3; i < (sbs - 2); i++) {
		checksum += *send_buffer;
		if (	(*send_buffer == STX ) || 
			(*send_buffer == ETX ) ||
			(*send_buffer == ENQ ) ||
			(*send_buffer == ACK ) ||	
			(*send_buffer == XOFF) ||
			(*send_buffer == XON ) ||
			(*send_buffer == NACK) ||
			(*send_buffer == ETB ) ||
			(*send_buffer == ESC )) {
			sb = g_renew (guchar, sb, ++sbs);
			sb[  i] = ESC;
			sb[++i] = ~*send_buffer;
		} else  sb[  i] =  *send_buffer;
		send_buffer++;
	}
	sb[sbs - 2] = ETX;
	checksum += ETX;
	if (	(checksum == STX ) || 
		(checksum == ETX ) || 
		(checksum == ENQ ) || 
		(checksum == ACK ) || 
		(checksum == XOFF) ||
		(checksum == XON ) || 
		(checksum == NACK) || 
		(checksum == ETB ) ||
		(checksum == ESC )) {
		sb = g_renew (guchar, sb, ++sbs);
		sb[sbs - 2] = ESC;
		sb[sbs - 1] = ~checksum;
	} else  sb[sbs - 1] =  checksum;
	for (i = 0; ; i++) {
		/************************/
		/* Write data as above.	*/
		/************************/
		if ((result = gp_port_write (device, sb, sbs)) != GP_OK) {
			g_free (sb);
			return (result);
		}
		if ((result = gp_port_read (device, &c, 1)) < 1) {
			g_free (sb);
			return (result);
		}
		switch (c) {
		case ACK:
			/******************************************************/
			/* ACK received. We can proceed.                      */
			/******************************************************/
			g_free (sb);
			/****************/
			/* Write EOT.	*/
			/****************/
			c = EOT;
			if ((result = gp_port_write (device, &c, 1)) != GP_OK) return (result);
			return (GP_OK);
		case NACK:
			/******************************************************/
			/* NACK received. We'll try up to three times.        */
			/******************************************************/
			if (i == 2) {
				g_free (sb);
				return (GP_ERROR_CORRUPTED_DATA);
			} else break;
		default:
			/******************************************************/
			/* Should not happen.                                 */
			/******************************************************/
			return (GP_ERROR_CORRUPTED_DATA);
		}
	}
}


gint 
l_receive (gp_port* device, guchar** rb, guint* rbs, guint timeout)
{
	guchar 		c, d;
	gboolean 	error_flag;
	guint 		i, j, rbs_internal;
	guchar 		checksum;
	gint 		result;

	g_return_val_if_fail (device, GP_ERROR_BAD_PARAMETERS);

	for (i = 0; ; ) {
		gp_port_timeout_set (device, timeout);
		if ((result = gp_port_read (device, &c, 1)) < 1) return (result);
		gp_port_timeout_set (device, DEFAULT_TIMEOUT);
		switch (c) {
		case ENQ:
			/******************************************************/
			/* ENQ received. We can proceed.                      */
			/******************************************************/
			break;
		case ACK:
			/******************************************************/
			/* ACK received. We'll try again at most ten times.   */
			/******************************************************/
			if (i == 9) {
				/**********************************************/
				/* The camera hangs! Although it could be     */
				/* that the camera accepts one of the         */
				/* commands                                   */
				/*  - KONICA_CANCEL,                          */
				/*  - KONICA_GET_IO_CAPABILITY, and           */
				/*  - KONICA_SET_IO_CAPABILITY,               */
				/* we can not get the camera back to working  */
				/* correctly.                                 */
				/**********************************************/
				return (GP_ERROR_CORRUPTED_DATA);
			}
			i++;
			break;
		default:
			/******************************************************/
			/* We'll dump this data until we get ENQ (then, we'll */
			/* proceed) or nothing any more (then, we'll report   */
			/* error).                                            */
			/******************************************************/
			for (;;) {
				if ((result = gp_port_read (device, &c, 1)) < 0) return (result); 
				if (c == ENQ) break;
			}
			break;
		}
		if (c == ENQ) break;
	}
	/**************/
	/* Write ACK. */
	/**************/
	if ((result = gp_port_write (device, "\6", 1)) != GP_OK) return (result);
	for (*rbs = 0; ; ) {
		for (j = 0; ; j++) {
			if ((result = gp_port_read (device, &c, 1)) < 1) return (result);
			switch (c) {
			case STX:
				/**********************************************/
				/* STX received. We can proceed.              */
				/**********************************************/
				break;
			default:
				/**********************************************/
				/* We'll dump this data and try again.        */
				/**********************************************/
				continue;
			}
			/******************************************************/
			/* Read 2 bytes for size (low order byte, high order  */
			/* byte) plus ESC quotes.                             */
			/******************************************************/
			result = l_esc_read (device, &c);
			if (result != GP_OK) return (result);
			checksum = c;
			result = l_esc_read (device, &d);
			if (result != GP_OK) return (result);
			checksum += d;
			rbs_internal = (d << 8) | c;
			if (*rbs == 0) *rb = g_new (guchar, rbs_internal);
			else *rb = g_renew (guchar, *rb, *rbs + rbs_internal);
			/******************************************************/
			/* Read 'rbs_internal' bytes data plus ESC quotes.    */
			/******************************************************/
			error_flag = FALSE;
			for (i = 0; i < rbs_internal; i++) {
				result = l_esc_read (device, &((*rb)[*rbs + i]));
				switch (result) {
				case GP_ERROR_CORRUPTED_DATA:
					/**************************************/
					/* We already received ETX or ETB.    */
					/* Later, we'll read the checksum     */
					/* plus ESC quotes and reject the     */
					/* packet.                            */
					/**************************************/
					error_flag = TRUE;
        	                        break;
				case GP_OK:
					/**************************************/
					/* We can proceed.                    */
					/**************************************/
					checksum += (*rb)[*rbs + i];
					break;
				default:
					return (result);
				}
				if (error_flag) break;
			}
			if (!error_flag) {
				if ((result = gp_port_read (device, &d, 1)) < 1) return (result);
				switch (d) {
				case ETX:
					/**************************************/
					/* ETX received. This is the last     */
					/* packet.                            */
					/**************************************/
					break;
				case ETB:
					/**************************************/
					/* ETB received. There are more       */
					/* packets to come.                   */
					/**************************************/
					break;
				default:
					/**************************************/
					/* We get more bytes than expected.   */
					/* Nothing serious, as we will simply */
					/* dump all bytes until we receive    */
					/* ETX or ETB. Later, we'll read the  */
					/* checksum with ESC quotes and       */
					/* reject the packet.                 */
					/**************************************/
					while ((d != ETX) && (d != ETB)) {
						if ((result = gp_port_read (device, &d, 1)) < 1) return (result);
					}
					error_flag = TRUE;
					break;
				}
			}
			checksum += d;
			/******************************************************/
			/* Read 1 byte for checksum plus ESC quotes.          */
			/******************************************************/
			if ((result = l_esc_read (device, &c)) != GP_OK) return (result);
			if ((c == checksum) && (!error_flag)) {
				*rbs += rbs_internal;
				/****************/
				/* Write ACK.	*/
				/****************/
				if ((result = gp_port_write (device, "\6", 1)) != GP_OK) return (result);
				break;
			} else {
				/**********************************************/
				/* Checksum wrong or transmission error. The  */
				/* camera will send us the data at the most   */
				/* three times.                               */
				/**********************************************/
				if (j == 2) return (GP_ERROR_CORRUPTED_DATA);
				/****************/
				/* Write NACK.	*/
				/****************/
				c = NACK;
				if ((result = gp_port_write (device, &c, 1)) != GP_OK) return (result);
				continue;
			}
		}
		if ((result = gp_port_read (device, &c, 1)) < 1) return (result);
		switch (c) {
			case EOT:
				/**********************************************/
				/* EOT received. We can proceed.              */
				/**********************************************/
				break;
			default:
				/**********************************************/
				/* Should not happen.                         */
				/**********************************************/
				return (GP_ERROR_CORRUPTED_DATA);
		}
		/**************************************************************/
		/* Depending on d, we will either continue to receive data or */
		/* stop.						      */
		/*  - ETX:  We are all done. 				      */
		/*  - ETB:  We expect more data.			      */
		/*  - else: Should not happen.				      */
		/**************************************************************/
		switch (d) {
		case ETX:
			/******************************************************/
			/* We are all done.				      */
			/******************************************************/
			return (GP_OK);
		case ETB:
			/******************************************************/
			/* We expect more data.                               */
			/******************************************************/
			/****************/
			/* Read ENQ.	*/
			/****************/
			if ((result = gp_port_read (device, &c, 1)) < 1) return (result);
			switch (c) {
			case ENQ:
				/**********************************************/
				/* ENQ received. We can proceed.              */
				/**********************************************/
				break;
			default:
				/**********************************************/
				/* Should not happen.                         */
				/**********************************************/
				return (GP_ERROR_CORRUPTED_DATA);
			}
			/****************/
			/* Write ACK.	*/
			/****************/
			if ((result = gp_port_write (device, "\6", 1)) != GP_OK) return (result);
			break;
		default:
			/******************************************************/
			/* Should not happen.                                 */
			/******************************************************/
			return (GP_ERROR_CORRUPTED_DATA);
		}
	}

}


gint 
l_send_receive (
	gp_port*	device,
	guchar*		send_buffer, 
	guint		send_buffer_size, 
	guchar**	receive_buffer, 
        guint*		receive_buffer_size,
	guint		timeout, 
	guchar**        image_buffer,
	guint*          image_buffer_size)
{
	gint result;

	if (timeout == 0) timeout = DEFAULT_TIMEOUT;

	/* Send data. */
        result = l_send (device, send_buffer, send_buffer_size);
        if (result != GP_OK) return (result);
	
        /* Receive data. */
        result = l_receive (device, receive_buffer, receive_buffer_size, timeout);
        if (result != GP_OK) return (result);
	
	/* Check if we've received the control data. */
	if (((*receive_buffer)[0] == send_buffer[0]) && ((*receive_buffer)[1] == send_buffer[1])) return (GP_OK);
	*image_buffer = *receive_buffer; 
	*image_buffer_size = *receive_buffer_size;
	*receive_buffer = NULL;
	
	/* Receive control data. */
        result = l_receive (device, receive_buffer, receive_buffer_size, DEFAULT_TIMEOUT);
	if (result != GP_OK) return (result);

	/* Sanity check: Did we receive the right control data? */
	g_return_val_if_fail (((*receive_buffer)[0] == send_buffer[0]) && ((*receive_buffer)[1] == send_buffer[1]), GP_ERROR_CORRUPTED_DATA);
	
	return (GP_OK);
}
