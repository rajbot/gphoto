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

#include "src/gphoto.h"

#include "serial.h"
#include "util.h"
#include "crc.h"
#include "psa50.h"


/* ------------------------- Frame-level procssing ------------------------- */


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
    if (type == PKT_EOT || type == PKT_ACK) len = 2; /* @@@ hack */
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
    int raw_length,length;

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
	    return NULL;
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
    return pkt+PKT_HDR_LEN;
}


/* ----------------------- Message-level processing ------------------------ */


#define MAX_MSG_SIZE	(MAX_PKT_PAYLOAD-12)

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


static unsigned char psa50_eot[8];


static int psa50_wait_for_ack(void)
{
    unsigned char *pkt;
    unsigned char type,seq;
    int len;

    while (1) {
	pkt = psa50_recv_packet(&type,&seq,&len);
	if (!pkt) return 0;
	if (seq == seq_tx && type == PKT_ACK) {
	    seq_tx++;
	    return 1;
	}
	fprintf(stderr,"ERROR: ACK format or sequence error, retrying\n");
/*
 * just keep on trying. protocol seems to retransmit EOTs, so we may get
 * some old EOTs when we're actually expecting ACKs.
 */
    }
}


static int psa50_send_msg(unsigned char mtype,unsigned char dir,
  const unsigned char *fffb,va_list *ap)
{
    unsigned char buffer[MAX_PKT_PAYLOAD+2]; /* allow space for CRC */
    unsigned char *pkt,*pos;
    int total;

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
	    fprintf(stderr,"FATAL ERROR: message too big\n");
	    exit(1);
	}
	memcpy(pos,str,len);
	pos += len;
    }
    total = pos-pkt;
    pkt[MSG_LEN_LSB] = total & 0xff;
    pkt[MSG_LEN_MSB] = total >> 8;
    if (!psa50_send_packet(PKT_MSG,0,pkt,total)) return 0;
    if (!psa50_send_packet(PKT_EOT,seq_tx,psa50_eot+PKT_HDR_LEN,1)) return 0;
    return psa50_wait_for_ack();
}


static unsigned char *psa50_recv_msg(unsigned char mtype,unsigned char dir,
    const unsigned char *fffb,int *total)
{
    static unsigned char *msg = NULL;
    static int msg_size = 512; /* initial allocation/2 */
    unsigned char *frag;
    unsigned char type,seq;
    int len,length,msg_pos = 0;

    while (1) {
	frag = psa50_recv_packet(&type,NULL,&len);
	if (!frag) return NULL;
	if (type == PKT_MSG) break;
	fprintf(stderr,"ERROR: protocol error, retrying\n");
    }
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
    while (1) {
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
	frag = psa50_recv_packet(&type,&seq,&len);
	if (!frag) return NULL;
	if (type == PKT_EOT) {
	    if (seq == seq_rx)  break;
	    fprintf(stderr,"ERROR: out of sequence\n");
	    return NULL;
	}
	if (type != PKT_MSG) {
	    fprintf(stderr,"ERROR: unexpected packet type\n");
	    return NULL;
	}
    }
    if (!psa50_send_packet(PKT_ACK,seq_rx++,psa50_eot+PKT_HDR_LEN,0))
	return NULL;
    if (total) *total = msg_pos;
    return msg;
}


static unsigned char *psa50_dialogue(unsigned char mtype,unsigned char dir,
  const unsigned char *fffb,int *len,...)
{
    va_list ap;
    int okay;

    va_start(ap,len);
    okay = psa50_send_msg(mtype,dir,fffb,&ap);
    va_end(ap);
    if (!okay) return NULL;
    return psa50_recv_msg(mtype,dir ^ DIR_REVERSE,fffb,len);
}


/* ----------------------- Command-level processing ------------------------ */


#define MAX_TRIES 10


char psa50_id[200]; /* some models may have a lot to report */


int psa50_ready()
{
    static int active = 0;
    unsigned char type,seq;
    char *pkt;
    int try,len;

    serial_set_timeout(1);
    serial_flush_input();
    if (active) {
	if (!psa50_send_packet(PKT_EOT,seq_tx,psa50_eot+PKT_HDR_LEN,0)) {
	    active = 0;
	    return 0;
	}
	if (psa50_wait_for_ack()) {
	    serial_set_timeout(10);
	    return 1;
	}
	active = 0;
    }
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

    /* printf("psa50_id : '%s'\n",psa50_id));*/

    if (!strcmp("DE300 Canon Inc.",psa50_id)) A5 = 1;
    else A5 = 0;

    /* printf("A5  %d in psa50 \n",A5));   */

    serial_set_timeout(10);
    (void) psa50_recv_packet(&type,&seq,NULL);
    if (type != PKT_EOT || seq) {
	update_status("Bad EOT");
	return 0;
    }
    seq_tx = 0;
    seq_rx = 1;
    if (!psa50_send_frame("\x00\x05\x00\x00\x00\x00\xdb\xd1",8) ||
      !psa50_send_frame("\x00\x03\x02\x02\x01\x08\x00\x00\x00\x00\xa0\xd7",12)
      || !psa50_send_frame("\x00\x04\x01\x00\x00\x00\x24\xc6",8)) {
	update_status("Communication error");
	return 0;
    }
    if (!psa50_wait_for_ack()) return 0;
    update_status("OK");
    active = 1;
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
fprintf(stderr,"\"%s\", %d bytes, %s",dir[entries].name,dir[entries].size,
  ctime(&dir[entries].date));
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
