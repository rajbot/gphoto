/*
 * psa50.c - Canon PowerShot A50 "native" operations.
 *
 * Written 1999 by Wolfgang G. Reissnegger and Werner Almesberger
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>

#include "src/gphoto.h"


#include "serial.h"
#include "util.h"
#include "crc.h"
#include "psa50.h"


#define MAX_TRIES 10

#define NOERROR			0
#define ERROR_RECEIVED	1 
#define ERROR_ADDRESSED	2

static unsigned char psa50_eot[8];
static int receive_error;

/* ------------------------- Frame-level processing ------------------------- */


#define CANON_FBEG	0xc0
#define CANON_FEND	0xc1
#define CANON_ESC	0x7e
#define CANON_XOR	0x20


int psa50_send_frame(const unsigned char *pkt,int len)
{
    static unsigned char buffer[2100];
	/* worst case: two maximum-sized packets (~1020 bytes, full of data
	   that needs to be escaped */
    unsigned char *p;

    p = buffer;
    *p++ = CANON_FBEG;
    while (len--) {
	if (p-buffer >= sizeof(buffer)-1) {
	    fprintf(stderr,"FATAL ERROR: send buffer overflow\n");
	    exit(1);
	}
	if (*pkt != CANON_FBEG && *pkt != CANON_FEND && *pkt != CANON_ESC)
	    *p++ = *pkt++;
	else {
	    *p++ = CANON_ESC;
	    *p++ = *pkt++ ^ CANON_XOR;
	}
    }
    *p++ = CANON_FEND;
    return !canon_serial_send(buffer,p-buffer);
}


unsigned char *psa50_recv_frame(int *len)
{
    static unsigned char buffer[5000];
	/* more than enough :-) (allow for a few run-together packets) */
    unsigned char *p = buffer;
    int c;

    while ((c = canon_serial_get_byte()) != CANON_FBEG)
	if (c == -1) return NULL;
    while ((c = canon_serial_get_byte()) != CANON_FEND) {
	if (c < 0) return NULL;
	if (c == CANON_ESC) c = canon_serial_get_byte() ^ CANON_XOR;
	if (p-buffer >= sizeof(buffer)) {
	    fprintf(stderr,"FATAL ERROR: receive buffer overflow\n");
	    exit(1);
	}
	*p++ = c;
    }
    dump_hex("RECV",buffer,p-buffer);
    if (len) *len = p-buffer;
    return buffer;
}


/* ------------------------ Packet-level procssing ------------------------- */


#define MAX_PKT_PAYLOAD 65535
#define MAX_PKT_SIZE	(MAX_PKT_PAYLOAD+4)

#define PKT_HDR_LEN	4

#define PKT_SEQ		0
#define PKT_TYPE	1
#define PKT_LEN_LSB	2
#define PKT_LEN_MSB	3

#define PKT_MSG		0
#define PKT_EOT		4
#define PKT_ACK		5
#define PKT_NACK	255	
#define PKTACK_NACK	0x01

static unsigned char seq_tx = 1;
static unsigned char seq_rx = 1;


static int psa50_send_packet(unsigned char type,unsigned char seq,
  unsigned char *pkt,int len)
{
    unsigned char *hdr = pkt-PKT_HDR_LEN;
    unsigned short crc;

	
    hdr[PKT_TYPE] = type;
    hdr[PKT_SEQ] = seq;
    hdr[PKT_LEN_LSB] = len & 0xff;
    hdr[PKT_LEN_MSB] = len >> 8;
	if (type == PKT_NACK) {
		hdr[PKT_TYPE] = PKT_ACK;
		hdr[PKT_TYPE+1] = '\xff';/* PKTACK_NACK; */
	}
    if (type == PKT_EOT || type == PKT_ACK || type == PKT_NACK) len = 2; /* @@@ hack */
    crc = canon_psa50_gen_crc(hdr,len+PKT_HDR_LEN);
    pkt[len] = crc & 0xff;
    pkt[len+1] = crc >> 8;
    return psa50_send_frame(hdr,len+PKT_HDR_LEN+2);
}


static unsigned char *psa50_recv_packet(unsigned char *type,unsigned char *seq,
  int *len)
{
    unsigned char *pkt;
    unsigned short crc;
    int raw_length,length=0;

    pkt = psa50_recv_frame(&raw_length);
    if (!pkt) return NULL;
    if (raw_length < PKT_HDR_LEN) {
	fprintf(stderr,"ERROR: packet truncated\n");
	return NULL;
    }
    if (pkt[PKT_TYPE] == PKT_MSG) {
	length = pkt[PKT_LEN_LSB] | (pkt[PKT_LEN_MSB] << 8);
	if (length+PKT_HDR_LEN > raw_length-2) {
	    fprintf(stderr,"ERROR: invalid length\n");
		/*fprintf(stderr,"Sending NACK\n");
		psa50_send_packet(PKT_NACK,seq_rx++,psa50_eot+PKT_HDR_LEN,0); */
		receive_error = ERROR_RECEIVED;
	    return "error";
	}
    }
    crc = pkt[raw_length-2] | (pkt[raw_length-1] << 8);
    if (!canon_psa50_chk_crc(pkt,raw_length-2,crc)) {
	fprintf(stderr,"ERROR: CRC error\n");
	return NULL;
    }
    if (type) *type = pkt[PKT_TYPE];
    if (seq) *seq = pkt[PKT_SEQ];
    if (len) *len = length;
	if (*type == PKT_ACK || *type == PKT_EOT) return pkt;
    return pkt+PKT_HDR_LEN; 
}


/* ----------------------- Message-level processing ------------------------ */


#define MAX_MSG_SIZE	(MAX_PKT_PAYLOAD-12)

#define FRAG_NUM		0
#define FRAG_LEN_LSB	2
#define FRAG_LEN_MSB	3

#define MSG_HDR_LEN	16
#define MSG_02		0
#define MSG_MTYPE	4
#define MSG_DIR		7
#define MSG_LEN_LSB	8
#define MSG_LEN_MSB	9
#define MSG_FFFB	12



#define DIR_REVERSE	0x30




/**
 * Waits for an "ACK" from the camera.
 *
 * Return values:
 *  1 : ACK received
 *  0 : communication error (no reply received for example)
 * -1 : NACK received.
 */
static int psa50_wait_for_ack(void)
{
    unsigned char *pkt;
    unsigned char type,seq,old_seq;
    int len;

    while (1) {
	pkt = psa50_recv_packet(&type,&seq,&len);
	if (!pkt) return 0; 
	if (seq == seq_tx && type == PKT_ACK) {
		if (pkt[2] == PKTACK_NACK) {
			fprintf(stderr,"ERROR: NACK received\n");
			return -1;
		} 
	    seq_tx++;
	    return 1;
	}
	old_seq='\0';
	if (type == PKT_EOT) {
		old_seq=pkt[0];
		if (receive_error == NOERROR) {
		fprintf(stderr,"Old EOT received, sending corresponding ACK\n");
		if (!psa50_send_packet(PKT_ACK,old_seq,psa50_eot+PKT_HDR_LEN,0)) return 0;
		pkt = psa50_recv_packet(&type,&seq,&len);
		if (!pkt) return 0;
		if (seq == old_seq && type == PKT_ACK) {
			if (pkt[2] == PKTACK_NACK) {
				fprintf(stderr,"Old EOT acknowledged\n");
				return -1;
			} 
	    	return 1;
		}
	}
	}
	/* error already aknowledged, we skip the following ones */
	if (receive_error == ERROR_RECEIVED) {
		if (!psa50_send_packet(PKT_NACK,old_seq,psa50_eot+PKT_HDR_LEN,0)) return 0;
		return 1;
	}
	
	fprintf(stderr,"ERROR: ACK format or sequence error, retrying\n"); 
	fprintf(stderr,"Sending NACK\n");
	psa50_send_packet(PKT_NACK,seq_rx++,psa50_eot+PKT_HDR_LEN,0);
	receive_error = ERROR_RECEIVED;
	
/*
 * just keep on trying. protocol seems to retransmit EOTs, so we may get
 * some old EOTs when we're actually expecting ACKs.
 */
    }
}

/**
 * Sends a message to the camera.
 *
 * See the "Protocol" file for an explanation of the various
 * elements needed to create a message.
 *
 * Arguments:
 *  mtype : message type.
 *  dir   : direction.
 *  fffb  : message operation
 *  ap    : message payload (list of arguments, see 'man va_start'
 */
static int psa50_send_msg(unsigned char mtype,unsigned char dir,
  const unsigned char *fffb,va_list *ap)
{
    unsigned char buffer[MAX_PKT_PAYLOAD+2]; /* allow space for CRC */
    unsigned char *pkt,*pos;
    int total, good_ack, try;

    memset(buffer,0,PKT_HDR_LEN+MSG_HDR_LEN);
    pkt = buffer+PKT_HDR_LEN;
    pkt[MSG_02] = 2;
    pkt[MSG_MTYPE] = mtype;
    pkt[MSG_DIR] = dir;
    memcpy(pkt+MSG_FFFB,fffb,4);
    pos = pkt+MSG_HDR_LEN;
    total = 0;
    while (1) {
	const char *str;
	int len;

	str = va_arg(*ap,unsigned char *);
	if (!str) break;
	len = va_arg(*ap,int);
	if (pos+len-pkt > MAX_MSG_SIZE) {
	  fprintf(stderr,"FATAL ERROR: message too big (%i)\n",pos+len-pkt);
	  exit(1);
	}
	memcpy(pos,str,len);
	pos += len;
    }
    total = pos-pkt;
    pkt[MSG_LEN_LSB] = total & 0xff;
    pkt[MSG_LEN_MSB] = total >> 8;
	for (try=1; try < MAX_TRIES; try++) {
    	if (!psa50_send_packet(PKT_MSG,0,pkt,total)) return 0;
    	if (!psa50_send_packet(PKT_EOT,seq_tx,psa50_eot+PKT_HDR_LEN,1)) return 0;
		good_ack = psa50_wait_for_ack();
		if (good_ack == -1) {
			fprintf(stderr,"NACK received, retrying command\n");
		}
		else if (good_ack==1) {
			return good_ack;
		}
		else  {
			fprintf(stderr,"No ACK received, retrying command\n");
		}
	}
    return -1;
}


/**
 * Receive a message from the camera.
 *
 * See the "Protocol" file for an explanation of the various
 * elements needed to handle a message.
 *
 * Arguments:
 *  mtype : message type.
 *  dir   : direction.
 *  fffb  : message operation.
 *  total : payload length (set by this function).
 *
 * Returns:
 *  char* : pointer to the message payload.
 */
static unsigned char *psa50_recv_msg(unsigned char mtype,unsigned char dir,
    const unsigned char *fffb,int *total)
{
    static unsigned char *msg = NULL;
    static int msg_size = 512; /* initial allocation/2 */
    unsigned char *frag;
    unsigned char type,seq;
    int len,length = 0,msg_pos = 0;

    while (1) { 
		frag = psa50_recv_packet(&type,NULL,&len);
		if (!frag) return NULL;
		if (type == PKT_MSG) break;
		if (type == PKT_EOT) {
			fprintf(stderr,"Old EOT received sending corresponding ACK\n");
			psa50_send_packet(PKT_ACK,frag[0],psa50_eot+PKT_HDR_LEN,0);
		}
		fprintf(stderr,"ERROR: protocol error, retrying\n");
    } 
    /* we keep the fragment only if there was no error */ 
	if (receive_error == NOERROR) {
    length = frag[MSG_LEN_LSB] | (frag[MSG_LEN_MSB] << 8);
    if (len < MSG_HDR_LEN || frag[MSG_02] != 2) {
		fprintf(stderr,"ERROR: message format error\n");
		return NULL;
    }
    if (frag[MSG_MTYPE] != mtype || frag[MSG_DIR] != dir ||
      memcmp(frag+MSG_FFFB,fffb,4)) {
		fprintf(stderr,"ERROR: unexpected message\n");
		return NULL;
    }
    frag += MSG_HDR_LEN;
    len -= MSG_HDR_LEN;
	}
    while (1) {
		if (receive_error == NOERROR) {
	if (msg_pos+len > length) {
	    fprintf(stderr,"ERROR: message overrun\n");
	    return NULL;
	}
	if (msg_pos+len > msg_size || !msg) {
	    msg_size *= 2;
	    msg = realloc(msg,msg_size);
	    if (!msg) {
		perror("realloc");
		exit(1);
	    }
	}
	memcpy(msg+msg_pos,frag,len);
	msg_pos += len;
		}
	frag = psa50_recv_packet(&type,&seq,&len);
	if (!frag) return NULL;
	if (type == PKT_EOT) {
		/* in case of error we don't want to stop as the camera will send
			the 1st packet of the sequence again */
			if (receive_error == ERROR_RECEIVED) {
				seq_rx = seq;
				psa50_send_packet(PKT_NACK,seq_rx,psa50_eot+PKT_HDR_LEN,0);
				receive_error = ERROR_ADDRESSED;
			} else {
	    if (seq == seq_rx)  break;
	    fprintf(stderr,"ERROR: out of sequence\n");
	    return NULL;
	}
		}
		if (type != PKT_MSG && receive_error == NOERROR) {
	    fprintf(stderr,"ERROR: unexpected packet type\n");
	    return NULL;
	}
		if (type == PKT_EOT && receive_error == ERROR_RECEIVED) {
			receive_error = ERROR_ADDRESSED;
		}
		if (type == PKT_MSG && receive_error == ERROR_ADDRESSED) {
			msg_pos =0;
			length = frag[MSG_LEN_LSB] | (frag[MSG_LEN_MSB] << 8);
    		if (len < MSG_HDR_LEN || frag[MSG_02] != 2) {
				fprintf(stderr,"ERROR: message format error\n");
				return NULL;
    		}
    		if (frag[MSG_MTYPE] != mtype || frag[MSG_DIR] != dir ||
      	  	  memcmp(frag+MSG_FFFB,fffb,4)) {
				fprintf(stderr,"ERROR: unexpected message\n");
				return NULL;
    		}
    		frag += MSG_HDR_LEN;
    		len -= MSG_HDR_LEN;
			receive_error = NOERROR;
		}
    }
	if (receive_error == ERROR_ADDRESSED) {
		receive_error = NOERROR;
	}
	if (receive_error == NOERROR) {		
    if (!psa50_send_packet(PKT_ACK,seq_rx++,psa50_eot+PKT_HDR_LEN,0))
	return NULL;
    if (total) *total = msg_pos;
    return msg;
	}
	
	return NULL;
}

/**
 * Higher level function: sends a message and waits for a
 * reply from the camera.
 *
 * Arguments:
 *   mtype : type
 *   dir   : direction
 *   fffb  : operation
 *   len   : length of the received payload
 *   ...   : The rest of the arguments will be put together to
 *           fill up the payload of the request message.
 *
 *
 *  Payload: each argument after "len" goes by 2: the variable itself,
 * and the next argument has to be its length. You also have to finish
 * the list by a "NULL".
 *
 * Example: To send a string called "name" :
 * psa50_dialogue(0x05,0x12,"\xfc\xd2\x9d",&len,name,strlen(name)+1,NULL);
 *
 */
static unsigned char *psa50_dialogue(unsigned char mtype,unsigned char dir,
  const unsigned char *fffb,int *len,...)
{
    va_list ap;
    int okay,try;
	unsigned char *good_ack;

	for ( try = 1; try < MAX_TRIES; try++) {
		va_start(ap,len);
		okay = psa50_send_msg(mtype,dir,fffb,&ap);
		va_end(ap);
		if (!okay) return NULL;
		good_ack = psa50_recv_msg(mtype,dir ^ DIR_REVERSE,fffb,len);
		if (good_ack) return good_ack;
		if (receive_error == NOERROR) {
		fprintf(stderr,"Resending message\n");
		seq_tx--;
	}
	}
	return NULL;
}


/* ----------------------- Command-level processing ------------------------ */




#define JPEG_ESC        0xFF
#define JPEG_BEG        0xD8
#define JPEG_SOS        0xDB
#define JPEG_END        0xD9

#define SPEED_9600   "\x00\x03\x02\x02\x01\x10\x00\x00\x00\x00\xc0\x39"
#define SPEED_19200  "\x00\x03\x08\x02\x01\x10\x00\x00\x00\x00\x13\x1f"
#define SPEED_38400  "\x00\x03\x20\x02\x01\x10\x00\x00\x00\x00\x5f\x84"
#define SPEED_57600  "\x00\x03\x40\x02\x01\x10\x00\x00\x00\x00\x5e\x57"
#define SPEED_115200 "\x00\x03\x80\x02\x01\x10\x00\x00\x00\x00\x4d\xf9"

char psa50_id[200]; /* some models may have a lot to report */
struct canon_info camera;


/**
 * Switches the camera off, closes the serial driver and restores
 * the serial port to its previous settings.
 */
int psa50_end()
{
        canon_serial_send("\xC0\x00\x02\x55\x2C\xC1",6);
        canon_serial_send("\xC0\x00\x04\x01\x00\x00\x00\x24\xC6\xC1",8);
        canon_serial_restore(); 
        return 0;
}

/**
 * Switches the camera off, and resets the serial driver to 9600 bauds,
 * in order to be ready to switch the camera back on again if wanted.
 */
int psa50_off()
{
        canon_serial_send("\xC0\x00\x02\x55\x2C\xC1",6);
        canon_serial_send("\xC0\x00\x04\x01\x00\x00\x00\x24\xC6\xC1",8);
       canon_serial_change_speed(B9600);
        return 0;
}


/**
 * Synchronize camera time to Unix time.
 */
int psa50_sync_time()
{


  return 0;
}

/**  Gets   the   camera identification  string,  usually   the  owner
 * name.  This information is stored  in the "camera" structure, which
 * is a global variable for the driver.
 *
 */
int psa50_get_owner_name(void)
{
  unsigned char *msg;
  int len;

  msg =  psa50_dialogue(0x01,0x12,"\x14\xf7\x8a",&len,NULL);
  if (!msg) return 0;
  /* Store these values in our "camera" structure: */
  strncpy(camera.owner,(char *) msg+44,30);
  strncpy(camera.ident,(char *) msg+12,30);
  return 0;
}


/**  Sets   the   camera owner name. The string should
 * not be more than 30 characters long. We call get_owner_name
 * afterwards in order to check that everything went fine.
 *
 */
int psa50_set_owner_name(const char *name)
{
  unsigned char *msg;
  int len;

  if (strlen(name) > 30) {
      update_status("Name too long, could not store it !");
      return 0;
  }
  fprintf(stderr,"New owner: %s",name);
  msg =  psa50_dialogue(0x05,0x12,"\xfc\xd2\x9d",&len,name,strlen(name)+1,NULL);

  if (!msg) return 0;

  return psa50_get_owner_name();
}


/**
 * Get camera's current time.
 *
 * The camera gives time in little endian format, therefore we need
 * to swap the 4 bytes on big-endian machines.
 *
 * Nota: the time returned is not GMT but local time. Therefore,
 * if you use functions like "ctime", it will be translated to local
 * time _a second time_, and the result will be wrong. Only use functions
 * that don't translate the date into localtime, like "gmtime".
 */
time_t psa50_get_time(void)
{
  unsigned char *msg;
  int len;
  time_t date;
  
  msg=psa50_dialogue(0x03,0x12,"\x78\xf3\x64\x01",&len,NULL);
  if (!msg) return 0;

  /* Beware, this is very dirty, and might fail one day, if time_t
     is not a 4-byte value anymore. */
  memcpy(&date,msg+4,4);

  return byteswap32(date);
}


/**
 * Switches the camera on, detects the model and sets its speed.
 */
int psa50_ready()
{
    unsigned char type,seq;
    char *pkt;
    int try,len,speed,good_ack,res,cts;

    serial_set_timeout(1);
    serial_flush_input();
    serial_flush_output();

    /* First of all, we must check if the camera is already on */
    cts=canon_serial_get_cts();
    printf("cts : %i",cts);
    if (cts==1) { // CTS is 1 when the camera is on.

      update_status("Camera already on...");
      /* First case, the serial speed of the camera is the same as
       * ours, so let's try to send a ping packet : */
      if (!psa50_send_packet(PKT_EOT,seq_tx,psa50_eot+PKT_HDR_LEN,0))
       return 0;
      good_ack=psa50_wait_for_ack();
      fprintf(stderr,"good_ack = %i\n",good_ack);
      if (good_ack==0) {
       /* no answer from the camera, let's try
        * at the speed saved in the settings... */
       speed=camera.speed;
       if (speed!=B9600) {
         if(!canon_serial_change_speed(speed)) {
           update_status("Error changing speed.");
           fprintf(stderr,"speed changed.\n");
         }
       }
       if (!psa50_send_packet(PKT_EOT,seq_tx,psa50_eot+PKT_HDR_LEN,0))
         return 0;
       good_ack=psa50_wait_for_ack();
       if (good_ack==0) {
         update_status("Resetting protocol...");
         psa50_off();
         sleep(3); /* The camera takes a while to switch off */
         return psa50_ready();
       }
       if (good_ack==-1) {
	 fprintf(stderr,"Received a NACK !\n");
         return 0;
       }
       update_status("Camera OK.\n");
       return 1;
      }
      if (good_ack==-1) {
       fprintf(stderr,"Received a NACK !\n");
       return 0;
      }
      fprintf(stderr,"Camera replied to ping, proceed.\n");
      return 1;
    }

    /* Camera was off... */

    update_status("Looking for camera ...");
    update_progress(0);
    for (try = 1; try < MAX_TRIES; try++) {
      update_progress(try/(float) MAX_TRIES);
      if (canon_serial_send("\x55\x55\x55\x55\x55\x55\x55\x55",8) < 0) {
	update_status("Communication error");
	return 0;
      }
      pkt = psa50_recv_frame(&len);
      if (pkt) break;
    }
    if (try == MAX_TRIES) {
      update_status("No response from camera");
      return 0;
    }
    if (!pkt) {
      update_status("No response from camera");
      return 0;
    }
    if (len < 40 && strncmp(pkt+26,"Canon",5)) {
      update_status("Unrecognized response");
      return 0;
    }
    strcpy(psa50_id,pkt+26); /* @@@ check size */

    printf("psa50_id : '%s'\n",psa50_id);

    if (!strcmp("DE300 Canon Inc.",psa50_id)) {
      update_status("Powershot A5");
      camera.model = CANON_PS_A5;
      A5 = 1;
    } else if (!strcmp("Canon PowerShot A5 Zoom",psa50_id)) {
      update_status("Powershot A5 Zoom");
      camera.model = CANON_PS_A5_ZOOM;
      A5 = 1;
    } else if (!strcmp("Canon PowerShot A50",psa50_id)) {
      update_status("Detected a Powershot A50");
      camera.model = CANON_PS_A50;
      A5 = 0;
    } else {
      update_status("Detected a Powershot S10");
      camera.model = CANON_PS_S10;
      A5 = 0;
    }

    serial_set_timeout(5);
    (void) psa50_recv_packet(&type,&seq,NULL);
    if (type != PKT_EOT || seq) {
      update_status("Bad EOT");
      return 0;
    }
    seq_tx = 0;
    seq_rx = 1;
    if (!psa50_send_frame("\x00\x05\x00\x00\x00\x00\xdb\xd1",8))
      {        update_status("Communication error");
       return 0;
      }
    res=0;
    switch (camera.speed) {
    case B9600: res=psa50_send_frame(SPEED_9600,12); break;
    case B19200: res=psa50_send_frame(SPEED_19200,12); break;
    case B38400: res=psa50_send_frame(SPEED_38400,12); break;
    case B57600: res=psa50_send_frame(SPEED_57600,12); break;
    case B115200: res=psa50_send_frame(SPEED_115200,12); break;
    }

    if( !res || !psa50_send_frame("\x00\x04\x01\x00\x00\x00\x24\xc6",8)) {
      update_status("Communication error");
      return 0;
    }
    speed=camera.speed;
    update_status("Changing speed... wait...");
    if (!psa50_wait_for_ack()) return 0;
    if (speed!=B9600) {
      if(!canon_serial_change_speed(speed)) {
	update_status("Error changing speed");
	fprintf(stderr,"speed changed\n");
      }
    }
    for (try=1; try < MAX_TRIES; try++) {
      psa50_send_packet(PKT_EOT,seq_tx,psa50_eot+PKT_HDR_LEN,0);
      if (!psa50_wait_for_ack()) {
	update_status("Error waiting ACK during initialization retrying");
      }
      else {
	break;
      }
    }		
    if (try==MAX_TRIES) {
      update_status("Error waiting ACK during initialization");	
      return 0;
    }
    update_status("Connected to camera");
    /* Now is a good time to ask the camera for its owner
     * name (and Model String as well)  */
    psa50_get_owner_name();
    psa50_get_time();
    return 1;
}
	
static unsigned int get_int(const unsigned char *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}


char *psa50_get_disk(void)
{
    unsigned char *msg;
    int len;

    msg = psa50_dialogue(0x0a,0x11,"\xdc\xf7\x8a",&len,NULL);
    if (!msg) return NULL;
    msg = strdup((char *) msg+4); /* @@@ should check length */
    if (!msg) perror("strdup");
    return msg;
}


int psa50_disk_info(const char *name,int *capacity,int *available)
{
    unsigned char *msg;
    int len;

    msg = psa50_dialogue(0x09,0x11,"\xd8\xf7\x8a",&len,name,strlen(name)+1,
      NULL);
    if (!msg) return 0;
    if (len < 12) {
	fprintf(stderr,"ERROR: truncated message\n");
	return 0;
    }
    if (capacity) *capacity = get_int(msg+4);
    if (available) *available = get_int(msg+8);
    return 1;
}


void psa50_free_dir(struct psa50_dir *list)
{
    struct psa50_dir *walk;

    for (walk = list; walk->name; walk++) free((char *) walk->name);
    free(list);
}


struct psa50_dir *psa50_list_directory(const char *name)
{
    struct psa50_dir *dir = NULL;
    int entries = 0;
    unsigned char *msg,*p;
    int len;

    msg = psa50_dialogue(0xb,0x11,"\xa8\xf6\x8a",&len,"",1,name,strlen(name)+1,
      "\x00",2,NULL);
    if (!msg) return NULL;
    if (len < 16) {
	fprintf(stderr,"ERROR: malformed message\n");
	return NULL;
    }
    if (!msg[5]) return NULL;
    p = msg+15;
    if (p >= msg+len) goto error;
    for (; *p; p++)
	if (p >= msg+len) goto error;
    while (1) {
	unsigned char *start;
	int is_file;

//fprintf(stderr,"p %p msg+len %p len %d\n",p,msg+len,len);
	if (p == msg+len-1) {
	    if (msg[4]) break;
	    msg = psa50_recv_msg(0xb,0x21,"\xa8\xf6\x8a",&len);
	    if (!msg) goto error;
	    if (len < 5) goto error;
	    p = msg+4;
	}
	if (p+2 >= msg+len) goto error;
	is_file = p[1] == 0x20;
	if (p[1] == 0x10 || is_file) p += 11;
	else break;
	if (p >= msg+len) goto error;
	for (start = p; *p; p++)
	    if (p >= msg+len) goto error;
	dir = realloc(dir,sizeof(*dir)*(entries+2));
	if (!dir) {
	    perror("realloc");
	    exit(1);
	}
	dir[entries].name = strdup(start);
	if (!dir[entries].name) {
	    perror("strdup");
	    exit(1);
	}
	memcpy((unsigned char *) &dir[entries].size,start-8,4);
	memcpy((unsigned char *) &dir[entries].date,start-4,4);
	dir[entries].is_file = is_file;
	entries++;
    }
    if (dir) dir[entries].name = NULL;
    return dir;
error:
    fprintf(stderr,"ERROR: truncated message\n");
    if (dir) psa50_free_dir(dir);
    return NULL;
}


unsigned char *psa50_get_file(const char *name,int *length)
{
    unsigned char *file = NULL;
    unsigned char *msg;
    unsigned char name_len;
    unsigned int total = 0,expect = 0,size;
    int len;

    update_progress(0);
    name_len = strlen(name)+1;
    msg = psa50_dialogue(0x1,0x11,"\x6a\x08\x79\x04",&len,"\x00\x00\x00\x00",5,
      &name_len,1,"\x00",2,name,strlen(name)+1,NULL);
    while (msg) {
	if (len < 20 || get_int(msg)) {
	    fprintf(stderr,"ERROR: malformed message\n");
	    break;
	}
	if (!file) {
	    total = get_int(msg+4);
	    if (total > 2000000) { /* 2 MB images ? unlikely ... */
		fprintf(stderr,"ERROR: %d is too big\n",total);
		break;
	    }
	    file = malloc(total);
	    if (!file) {
		perror("malloc");
		break;
	    }
	    if (length) *length = total;
	}
	size = get_int(msg+12);
	if (get_int(msg+8) != expect || expect+size > total || size > len-20) {
	    fprintf(stderr,"ERROR: doesn't fit\n");
	    break;
	}
	memcpy(file+expect,msg+20,size);
	expect += size;
	update_progress(total ? expect/(float) total : 1);
	if ((expect == total) != get_int(msg+16)) {
	    fprintf(stderr,"ERROR: end mark != end of data\n");
	    break;
	}
	if (expect == total) return file;
    	msg = psa50_recv_msg(0x1,0x21,"\x6a\x08\x79\x04",&len);
    }
    free(file);
    return NULL;
}

unsigned char *psa50_get_thumbnail(const char *name,int *length)
{
    unsigned char *file = NULL;
    unsigned char *thumb;
    unsigned char *msg;
    unsigned char name_len;
    unsigned int total = 0,expect = 0,size;
    int len,i,j,in;


    update_progress(0);
    name_len = strlen(name)+1;
    msg = psa50_dialogue(0x1,0x11,"\xea\x0c\xb1\x02",&len,
		"\x01\x00\x00\x00\x00",5,
      &name_len,1,"\x00",2,name,strlen(name)+1,NULL);
    while (msg) {
	if (len < 20 || get_int(msg)) {
	    fprintf(stderr,"ERROR: malformed message\n");
	    break;
	}
	if (!file) {
	    total = get_int(msg+4);
	    if (total > 2000000) { /* 2 MB images ? unlikely ... */
		fprintf(stderr,"ERROR: %d is too big\n",total);
		break;
	    }
	    file = malloc(total);
	    if (!file) {
		perror("malloc");
		break;
	    }
	    if (length) *length = total;
	}
	size = get_int(msg+12);
	if (get_int(msg+8) != expect || expect+size > total || size > len-20) {
	    fprintf(stderr,"ERROR: doesn't fit\n");
	    break;
	}
	memcpy(file+expect,msg+20,size);
	expect += size;
	update_progress(total ? expect/(float) total : 1);
	if ((expect == total) != get_int(msg+16)) {
	    fprintf(stderr,"ERROR: end mark != end of data\n");
	    break;
	}
	if (expect == total) {

		/* we want to drop the exif header to get the thumbnail */
		thumb = malloc(total);
		if (!thumb) {
                	perror("malloc");
                	break;
            	}
		/* we skip the first FF D8 */
		i=3;
		j=0;
		in=0;
		while(i<total) {
			if (file[i]==JPEG_ESC) {
                               if (file[i+1]==JPEG_BEG &&
                                   file[i+3]== JPEG_SOS) {
					in=1;
				}
				else if (file[i+1]==JPEG_END) {
					in=0;
					thumb[j++]=file[i];
					thumb[j]=file[i+1];
					return thumb;
				}
			}
				
			if (in==1)
				thumb[j++]=file[i];
			i++;
			
		}
	}
    	msg = psa50_recv_msg(0x1,0x21,"\xea\x0c\xb1\x02",&len);
    }
    free(file);
    return NULL;
}

int psa50_delete_file(const char *name, const char *dir)
{
	unsigned char *msg;	
	int len;
	
	msg =  psa50_dialogue(0xd,0x11,"\x8c\xf4\x7b",&len,dir,strlen(dir)+1,
			      name,strlen(name)+1,NULL);
	
	if (!msg) return -1;
	
	return 0;
}
