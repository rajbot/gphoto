/* qcam_drv.c -- Library for programming with the Connectix QuickCam.
 * See the included documentation for usage instructions and details
 * of the protocol involved. */


/* Version 0.5, August 4, 1996 */
/* Version 0.7, August 27, 1996 */

/******************************************************************

Copyright (C) 1996 by Scott Laird for the BW part
Copyright (C) 1996 by Thomas 'Dent' Mirlacher for the COLOR part

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL SCOTT LAIRD BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <assert.h>

#include "qcam.h"

#ifdef linux
#include "qcam-Linux.h"
#else
#include "qcam-FreeBSD.h"
#endif

#define CONFIG_FILE "/home/scottf/gphoto/quickcam/qcam.conf"

/* Prototypes for static functions.  Externally visible functions
 * should be prototyped in qcam.h */

static int qc_handshake		(struct qcam *q, u_int val);
static int qc_handshake2	(struct qcam *q, u_int val);
static int qc_command		(struct qcam *q, int command);
static int qc_readparam		(struct qcam *q);
static int qc_readbytes_color	(struct qcam *q, u_char buffer[]);
static int qc_readbytes_bw	(struct qcam *q, u_char buffer[]);
static int qc_getportdir	(struct qcam *q);
static int qc_getstatus		(struct qcam *q);
static int qc_setscanmode	(struct qcam *q);
static int qc_loadram		(struct qcam *q);
static void qc_geteof		(struct qcam *q);


/******************************************************************/
/* Gets/sets the brightness.                                      */

int qc_getbrightness (struct qcam *q)
{
  return (q->brightness);
}

/******************************************************************/

int qc_setbrightness (struct qcam *q, int val)
{
  if (val >= 0 && val <= 255) {
    q->brightness=val;
    return (0);
  }

  return (1);
}

/******************************************************************/

int qc_getport (struct qcam *q)
{
  if (!q->port) {

#ifdef DEBUG
  fprintf (stderr, "(qc_getport) port not set\n");
#endif

  qc_probe (q);
  }

  return (q->port);
}

/******************************************************************/

int qc_setport (struct qcam *q, int val)
{
  if (val == 0x278 || val == 0x378 || val == 0x37B || val == 0) {
    q->port = val;

    return (0);
  }

  return (1);
}

/******************************************************************/
/* Gets/sets the contrast                                         */

int qc_getcontrast (struct qcam *q)
{
  return q->contrast;
}

/******************************************************************/

int qc_setcontrast (struct qcam *q, int val)
{
  if (val >= 0 && val <= 255) {
    q->contrast=val;

    return (0);
  }

  return (1);
}

/******************************************************************/
/* Gets/sets the hue                                              */

int qc_gethue (struct qcam *q)
{
  return (q->hue);
}

/******************************************************************/

int qc_sethue (struct qcam *q, int val)
{
  if (val >= 0 && val <= 255) {
    q->hue=val;

    return (0);
  }

  return (1);
}

/******************************************************************/

int qc_setblackbal (struct qcam *q, int val)
{
  if (qc_getversion(q) == COLOR_QUICKCAM) {	// setblackbal only works for
						// color CAMs

#ifdef DEBUG
    fprintf (stderr, "(setblackbal) Waiting to finish .");
#endif

    qc_command (q, CMD_COLOR_SetBlack); qc_command (q, val);

    do {					// wait for SetBlack to finish
      usleep (1);

#ifdef DEBUG
    fprintf (stderr, ".");
#endif

    } while (qc_getstatus (q) & 0x40);

#ifdef DEBUG
    fprintf (stderr, " done\n");
#endif

    q->blackbal=val;

    return (0);
  }
  else
    return (1);
  
}

/******************************************************************/
/* Gets/sets the white balance                                    */

int qc_getwhitebal (struct qcam *q)
{
  return (q->whitebal);
}

/******************************************************************/

int qc_setwhitebal (struct qcam *q, int val)
{
  if (val >= 0 && val <= 255) {
    q->whitebal=val;

    return (0);
  }

  return (1);
}

/******************************************************************/
/* Gets/sets the saturation                                       */

int qc_getsaturation (struct qcam *q)
{
  return (q->saturation);
}

/******************************************************************/

int qc_setsaturation (struct qcam *q, int val)
{
  if (val >= 0 && val <= 255) {
    q->saturation=val;

    return (0);
  }

  return (1);
}

/******************************************************************/
/* Gets/sets the resolution                                       */

void qc_getresolution (struct qcam *q, int *x, int *y)
{
  *x=q->width;
  *y=q->height;
}

/******************************************************************/

int qc_setresolution (struct qcam *q, int x, int y)
{
  if (!qc_setheight (q, y) && !qc_setwidth (q, x)) {
    return (0);
  }

#ifdef DEBUG
    fprintf (stderr, "(qc_setresolution) resolution not allowed\n");
#endif

    return (1);
}

/******************************************************************/

int qc_getheight (struct qcam *q)
{
  return (q->height);
}

/******************************************************************/

int qc_setheight (struct qcam *q, int y)
{
  if (y >= 0 && y <= 500) {
    q->height=y;

    return (0);
  }

  return (1);
}

/******************************************************************/

int qc_getwidth (struct qcam *q)
{
  return (q->width);
}

/******************************************************************/

int qc_setwidth (struct qcam *q, int x)
{
  if (x >= 2 && x <= 680 && x%2 == 0) {
    q->width=x;

    return (0);
  }

  return (1);
}

/******************************************************************/
/* Gets/sets the bit depth                                        */

int qc_getbpp (struct qcam *q)
{
  return (q->bpp);
}

/******************************************************************/

int qc_setbpp (struct qcam *q, int val)
{
  if (val == 4 || val == 6 || val == 24) {	// ToDo: 24 check if C || BW
    q->bpp=val;

    return (0);
  }

  return (1);
}

/******************************************************************/
/* Gets/sets the top of the captured picture                      */

int qc_gettop (struct qcam *q)
{
  if (q->top == -1) {
   if (qc_getversion (q) == COLOR_QUICKCAM) {
     if (q->color_mode == QC_COLOR_BILLION)
      return ((480-qc_getheight(q))/2+1);
     if (q->color_mode == QC_COLOR_MILLION)
      return ((240-qc_getheight(q))/2+1);
    }
  }

  return (q->top);
}

/******************************************************************/

int qc_settop (struct qcam *q, int val)
{
  if (val >= -1 && val <= 250) {
    q->top = val;

    return (0);
  }

  return (1);
}

/******************************************************************/
/* Gets/sets the left of the captured picture                     */

int qc_getleft (struct qcam *q)
{
  if (q->left == -1) {
   if (qc_getversion(q) == COLOR_QUICKCAM) {
     if (q->color_mode == QC_COLOR_BILLION) 
       return ((640-qc_getwidth(q))/2+20);
     if (q->color_mode == QC_COLOR_MILLION) 
       return ((320-qc_getwidth(q))/2+20);
    }
  }
    
  return (q->left);
}

/******************************************************************/

int qc_setleft (struct qcam *q, int val)
{
  if ((val % 2 == 0 && val >= 0 && val <= 340) || (val == -1)) {
    q->left = val;

    return (0);
  }

  return (1);
}

/******************************************************************/
/* Gets/sets the scale size of the picture                        */

int qc_gettransfer_scale (struct qcam *q)
{
  return (q->transfer_scale);
}

/******************************************************************/

int qc_settransfer_scale (struct qcam *q, int val)
{
  if (val == 1 || val == 2 || val == 4) {
    q->transfer_scale = val;

    return (0);
  }

  return (1);
}

/******************************************************************/

static int qc_getstatus (struct qcam *q)
{ qc_command (q, CMD_COLOR_SendStatus); 

  return (qc_readparam (q));
}

/******************************************************************/
/* Gets the version of the cam and cam ROM                        */

int qc_getversion (struct qcam *q)
{ int version_cam = 0,
    version_rom = 0;

  if (!q->cam_version) {
    if (qc_command (q, CMD_SendVersion) != CMD_SendVersion)
	return (0);

    version_cam = qc_readparam (q);
    version_rom = qc_readparam (q);

    version_cam = (version_cam & 0xf0) ? COLOR_QUICKCAM : BW_QUICKCAM;

#ifdef DEBUG  
    fprintf (stderr, "Camera Version: %s-QuickCam\n", (version_cam == 0x10) ? "Color" : "BW");
    fprintf (stderr, "Connector Version: %d\n", version_rom);
#endif

    q->cam_version = version_cam;
  }

  return (q->cam_version);
}

/******************************************************************/

int qc_calibrate (struct qcam *q)
{
  int value = 0;
#ifdef DEBUG
  int count = 0;
#endif

  if (qc_getversion(q) == BW_QUICKCAM) {
    qc_command (q, CMD_BW_AutoAdjustOffset);
    qc_command(q, 0x00);

    do {
      qc_command (q, CMD_BW_GetOffset); value = qc_readparam(q);

#ifdef DEBUG
      count++;
#endif

    } while (value == 0xff);

    q->whitebal = value;

#ifdef DEBUG
    fprintf(stderr, "%d loops to calibrate\n", count);
    fprintf(stderr, "Calibrated to %d\n", value);
#endif

  }

#ifdef DEBUG
  else
    fprintf (stderr, "(qc_calibrate) This is no BW qcam -> can't calibrate!");
#endif DEBUG

  return (value);
}

/******************************************************************/

int qc_forceunidir (struct qcam *q)
{
  q->port_mode = (q->port_mode & ~QC_FORCE_MASK) | QC_FORCE_UNIDIR;

  return (0);
}

/******************************************************************/
/* Read from the QuickCam init file.  By default this is          */
/* /usr/local/etc/qcam.conf.  This can be changed in the Makefile.*/
/* This routine sets the 'qcam' structure to the defaults in the  */
/* config file.                                                   */

int qc_initfile (struct qcam *q, char *file)
{
	/* gPhoto- quick rehash, to eventually allow for
	   GTK config dialog :) */

	qc_setport  (q, 0x378);
	qc_setheight(q, 240);
	qc_setwidth (q, 320);
	qc_setbpp   (q, 24);
	qc_setbrightness (q, 135);
	qc_setwhitebal   (q, 128);
	qc_setcontrast   (q, 104);
	qc_settop   (q, 1);
	qc_setleft  (q, 14);
	qc_settransfer_scale (q, 1);
	qc_setsaturation     (q, 100);
	qc_sethue   (q, 128);
	q->blackbal   = 200;
	q->color_mode = 888;
/*

  FILE *f;
  char buf[256];
  int dummy, dummy2;

  if (file == NULL || *file == 0)
    file = CONFIG_FILE;

  f = fopen(file, "r");

  if (f == NULL) {

#ifdef DEBUG
    fprintf(stderr, "(qc_initfile) Cannot open configuration file %s!", file);
#endif

    return (1);
  }

#define BADENTRY(s) fprintf(stderr, "illegal %s (%d) in file %s\n", s, dummy, file);

  while (fgets(buf,255,f)) {
    if (buf[0]=='#') ;					// comment
    else if (buf[0] == '\n') ;				// blank line
    else if (sscanf(buf, " port %x", &dummy) == 1) {
      if (qc_setport (q, dummy) == 1) BADENTRY("port");
    } else if (sscanf(buf, " height %d", &dummy) == 1) {
      if (qc_setheight(q, dummy) == 1) BADENTRY("height");
    } else if (sscanf(buf, " width %d", &dummy) == 1) {
      if (qc_setwidth(q, dummy) == 1) BADENTRY("width");
    } else if (sscanf(buf, " resolution %dx%d", &dummy, &dummy2) == 2) {
      if (qc_setresolution(q, dummy, dummy2) == 1) BADENTRY("resolution");
    } else if (sscanf(buf, " bpp %d", &dummy) == 1) {
      if (qc_setbpp(q, dummy) == 1) BADENTRY("bpp");
    } else if (sscanf(buf, " brightness %d", &dummy) == 1) {
      if (qc_setbrightness(q, dummy) == 1) BADENTRY("brightness");
    } else if (sscanf(buf, " whitebal %d", &dummy) == 1) {
      if (qc_setwhitebal(q, dummy) == 1) BADENTRY("whitebal");
    } else if (sscanf(buf, " contrast %d", &dummy) == 1) {
      if (qc_setcontrast(q, dummy) == 1) BADENTRY("contrast");
    } else if (sscanf(buf, " top %d", &dummy) == 1) {
      if (qc_settop(q, dummy) == 1) BADENTRY("top");
    } else if (sscanf(buf, " left %d", &dummy) == 1) {
      if (qc_setleft(q, dummy) == 1) BADENTRY("left");
    } else if (sscanf(buf, " transfer %d", &dummy) == 1 ||
	       sscanf(buf, " transfer_scale %d", &dummy) == 1) {
      if (qc_settransfer_scale(q, dummy) == 1) BADENTRY ("transfer_scale");
    } else if (sscanf(buf, " blackbal %d", &dummy) == 1) {
      q->blackbal = dummy;
    } else if (sscanf(buf, " saturation %d", &dummy) == 1) {
      if (qc_setsaturation(q, dummy) == 1) BADENTRY("saturation");
    } else if (sscanf(buf, " hue %d", &dummy) == 1) {
      if (qc_sethue(q, dummy) == 1) BADENTRY("hue");
    } else if (sscanf(buf, " color_mode %d", &dummy) == 1) {
      q->color_mode=dummy;
    }
  }

  fclose(f);

#ifdef DEBUG
  fprintf (stderr, "(qc_initfile)\n");
#endif
*/
  return (0);
}

/******************************************************************/
/* Initialize the QuickCam driver control structure. This is where*/
/* defaults are set for people who don't have a config file.      */

struct qcam *qc_init (void)
{
  struct qcam *q;

  q=malloc(sizeof(struct qcam));

  q->port	= 0x00;				// Port 0 == Autoprobe
  q->port_mode	= (QC_ANY | QC_NOTSET);		// let the driver check
  q->width	= 320;				// 320
  q->height	= 240;				// 240
  q->bpp	= 24;
  q->transfer_scale = 1;
  q->contrast	= 104;
  q->brightness	= 130;
  q->whitebal	= 120;
  q->blackbal	= 128;
  q->hue	= 128;
  q->saturation	= 130;
  q->color_mode	= QC_COLOR_MILLION;
  q->top	= -1;
  q->left	= -1;
  q->mode	= -1;
  q->speed	= 2;
  q->cam_version= 0;
  q->fd		= -1;

  q->lp_control.data=0;
  q->lp_control.bit.strobe = 1;
  q->lp_control.bit.autofeed = 1;
  q->lp_control.bit.pcack = 1;
  q->lp_control.bit.reset_n = 1;

#ifdef DEBUG
  fprintf (stderr, "(qc_init)\n");
#endif

  return (q);
}


/******************************************************************/
/* qc_open enables access to the port specified in q->port.       */
/* It takes care of locking and enabling I/O port access by       */
/* calling the appropriate routines.                              */
/*                                                                */
/* Returns 0 for success, 1 for opening error, 2 for locking      */
/* error, and 3 for qcam not found.                               */

int qc_open(struct qcam *q)
{
  if(q->port==0)
    if(qc_probe(q)) {

#ifdef DEBUG
      fprintf(stderr,"(qc_open) Qcam not found\n");
#endif

      return (3);
    }

  if(qc_lock(q)) {

#ifdef DEBUG
    fprintf(stderr,"(qc_open) Cannot lock qcam.\n");
#endif

    return (2);
  }

  if(enable_ports(q) == -1) {

#ifdef DEBUG
    fprintf(stderr,"(qc_open) Cannot open QuickCam -- permission denied.");
#endif

    return (1);
  }
  
#ifdef DEBUG
  fprintf (stderr, "(qc_open) Qcam opened successfully (0x%02x).\n", q->port);
#endif

  q->port_mode = qc_getportdir (q);
  qc_reset (q);

  if (qc_getversion (q) == COLOR_QUICKCAM) {
    qc_loadram (q);
    qc_setblackbal (q, q->blackbal);
  }

  return (0);
}


/******************************************************************/
/* qc_close closes and unlocks the driver.  You *need* to call    */
/* this, or lockfiles will be left behind and everything will be  */
/* screwed.							  */

int qc_close(struct qcam *q)
{
  qc_unlock(q);

  disable_ports(q);

  return (0);
}

/******************************************************************/
/* just a hook ... to be implemented				  */

static int qc_loadram (struct qcam *q)
{ 
  return (0);
}

/******************************************************************/
/* qc_command is probably a bit of a misnomer -- it's used to send
 * bytes *to* the camera.  Generally, these bytes are either commands
 * or arguments to commands, so the name fits, but it still bugs me a
 * bit.  See the documentation for a list of commands. */

static int qc_command (struct qcam *q, int command)
{
  int hi, lo;
  int cmd;

  write_lpdata (q, command);

  hi = qc_handshake (q, 1);
  lo = qc_handshake (q, 0);

  cmd = (hi & 0xf0) | ((lo & 0xf0) >> 4);

#ifdef DEBUG
  if (cmd != command) {
    fprintf(stderr, "(qc_command) Command 0x%02x sent, 0x%02x echoed ... ", command, cmd);

    lo = read_lpstatus (q);
    cmd = ( hi& 0xf0) | ((lo & 0xf0) >> 4);

    if (cmd != command) 
	fprintf (stderr, "reread doesn't help\n");
    if (cmd == command) 
	fprintf (stderr, "fixed on reread\n");
  }
#endif

  return (cmd);
}

/******************************************************************/
/* read parameters sent *from* camera                             */

static int qc_readparam(struct qcam *q)
{
  int hi, lo;
  int cmd;

  hi = qc_handshake (q, 1);
  lo = qc_handshake (q, 0);

  cmd = (hi & 0xf0) | ((lo & 0xf0) >> 4);

  return (cmd);
}

/******************************************************************/
/* qc_handshake busy-waits for a handshake signal from the QuickCam*/
/* Almost all communication with the camera requires handshaking. */

static int qc_handshake (struct qcam *q, u_int val)
{
  u_int status=0;
  int i = 32000;

  q->lp_control.bit.pcack = ~val & 0x01;
  write_lpcontrol (q, q->lp_control.data);

  val <<= 3;
  while (i-- && (((status = read_lpstatus(q)) & 0x08) != val));

  /*if (val)
    while(! ((status = read_lpstatus(q))&8)) ;

  else
    while (((status = read_lpstatus(q))&8)) ;
*/
  return (i ? status : -1);
}

/******************************************************************/
/* handshake2 is used when the qcam is in bidirectional mode, and  */
/* the handshaking signal is CamRdy2 (bit 0 of data reg) instead  */
/* of CamRdy1 (bit 3 of status register). It also returns the last*/
/* value read, since this data is useful.                         */

static int qc_handshake2 (struct qcam *q, u_int val)
{
  u_int status;
  int i = 20000;

  q->lp_control.bit.pcack = ~val & 0x01;
  write_lpcontrol (q, q->lp_control.data);

  while (i-- && (((status = read_lpstatus_data(q)) & 0x01 ) != val));

  return (i ? status : -1);
}

/******************************************************************/
/* Try to detect a QuickCam.  It appears to flash the upper 4 bits*/
/* of the status register at 5-10 Hz.  This is only used in the   */
/* autoprobe code.  Be aware that this isn't the way Connectix    */
/* detects the camera (they send a reset and try to handshake),   */
/* but this should be almost completely safe, while their method  */
/* screws up my printer if I plug it in before the camera.	  */

int qc_detect(struct qcam *q)
{
  int reg,lastreg;
  int count=0;
  int i;

#ifdef DEBUG
  fprintf (stderr, "(qc_detect) 0x%02X\n", q->port);
#endif

  lastreg=reg=read_lpstatus(q)&0xf0;

  for(i=0;i<30;i++) {
    reg=read_lpstatus(q)&0xf0;
    if(reg!=lastreg) count++;
    lastreg=reg;
    usleep(10000);
  }


  if(count>3&&count<15)
    return (0); 				// found with 'hartbeat-way'
  else 						// not found, try the
						// 'connectix way'
    { q->cam_version = 0;			// make shure, we're getting
						// the version from the HW
      qc_reset (q);
      if ((qc_getversion (q) == COLOR_QUICKCAM) || 
          (qc_getversion (q) == BW_QUICKCAM)) {
        return (0);
      }
    }

  return (1);					// not found

}


/******************************************************************/
/* Reset the QuickCam.						  */

void qc_reset (struct qcam *q)
{
  q->lp_control.bit.reset_n = 0;
  //q->lp_control.bit.strobe = 1;
  qc_handshake (q, 1);
  //qc_handshake (q, 0);

  q->lp_control.bit.reset_n = 1;
  //q->lp_control.bit.strobe = 0;
  qc_handshake (q, 0);

//  qc_command (q, 0x00);

#ifdef DEBUG
  fprintf (stderr, "(qc_reset) cam resetted.\n");
#endif
}

/******************************************************************/
/* set SendFrameMode                                              */

static int qc_setscanmode(struct qcam *q)
{ int transfer_scale,
      port_mode,
      bpp,
      color_mode;

  switch (q->port_mode & QC_MODE_MASK) {
  case QC_BIDIR:
    port_mode = 1;
    break;

  case QC_NOTSET: 
  case QC_UNIDIR:
    port_mode = 0;
    break;

  default:
    return (1);
  }

  transfer_scale = q->transfer_scale >> 1;

  if (qc_getversion(q)== BW_QUICKCAM) {
    bpp = (q->bpp == 4) ? 0 : 1;

    q->mode = (port_mode & 1) |			// 0000 000X
              ((bpp & 1) << 1) |		// 0000 00X0
              ((transfer_scale & 3) << 2);	// 0000 XX00
  }
  else if (qc_getversion(q) == COLOR_QUICKCAM) {
    color_mode = (q->color_mode == QC_COLOR_MILLION) ? 3 : 2;
    
    q->mode = ((port_mode & 0x01) |		// 0000 000X
	      ((transfer_scale & 0x03) << 1) |	// 0000 0XX0
	      ((color_mode & 0x03) << 3)) |	// 000X X000
              0x80;				
    
#ifdef DEBUG
    fprintf (stderr, "(qc_setscanmode) 0x%02X\n", q->mode);
#endif

    q->mode++;
  }

#ifdef DEBUG
  else 
    fprintf (stderr, "(qc_setscanmode) Unknown QuickCam Version!\n");
#endif
     
  return (0);
}


/******************************************************************/
/* Set the QuickCam for brightness, contrast, white-balance, and  */
/* resolution.			                                  */

void qc_set (struct qcam *q)
{
  int val, val2;

#ifdef DEBUG
  fprintf (stderr, "(qc_set)\n");
#endif

  qc_setscanmode (q);

  if (qc_getversion(q) == BW_QUICKCAM) {
    qc_reset (q);

    qc_command (q, CMD_SetNumV);	qc_command (q, q->height / q->transfer_scale);

    if ((q->port_mode & QC_MODE_MASK) == QC_UNIDIR && q->bpp == 6) {
      val = q->width;
      val2 = q->transfer_scale * 4;
    }
    else {
      val = q->width * q->bpp;
      val2 = (((q->port_mode & QC_MODE_MASK) == QC_BIDIR)?24:8) * q->transfer_scale;
    }
 
    val = (val + val2 - 1) / val2;
    qc_command (q, CMD_SetNumH);	qc_command (q, val);

    qc_command (q, CMD_BW_SetContrast);	qc_command (q,q->contrast);
    qc_command (q, CMD_BW_SetOffset);	qc_command (q,q->whitebal);
  }
  else if (qc_getversion(q) == COLOR_QUICKCAM) {
    if (q->color_mode == QC_COLOR_MILLION) {
      qc_command (q, CMD_SetNumV);	qc_command (q, q->height);
      qc_command (q, CMD_SetNumH);	qc_command (q, q->width/2);
    }
    else if (q->color_mode == QC_COLOR_BILLION) {
      qc_command (q, CMD_SetNumV);	qc_command (q, q->height/2);
      qc_command (q, CMD_SetNumH);	qc_command (q, q->width/4);
    }
    qc_command (q, CMD_COLOR_SetSpeed);	qc_command (q, q->speed);
    qc_command (q, CMD_COLOR_SetHue);	qc_command (q, q->hue);
    qc_command (q, CMD_COLOR_SetSaturation); qc_command (q, q->saturation);
    qc_command (q, CMD_COLOR_SetContrast); qc_command (q, q->contrast);
    qc_command (q, CMD_COLOR_SetWhite);	qc_command (q, q->whitebal);
  }
  else
    fprintf (stderr, "(qc_set) Unknown QuickCam Version!\n");

  qc_command (q, CMD_SetBrightness);	qc_command (q, q->brightness);
  qc_command (q, CMD_SetTop);		qc_command (q, qc_gettop(q));
  qc_command (q, CMD_SetLeft);		qc_command (q, qc_getleft(q)/2);
}


/******************************************************************/
/* Qc_readbytes reads some bytes from the QC and puts them in     */
/* the supplied buffer.  It returns the number of bytes read,     */
/* or -1 on error.                                                */

static int __inline__ qc_readbytes_color (struct qcam *q, u_char buffer[])
{
  u_int hi, lo;
  u_int hi2, lo2;
  static int state = 0;

  if (!buffer) {				// reset state
    state = 0;
    return (0);
  }

  switch (q->port_mode & QC_MODE_MASK) {
  case QC_BIDIR:  				// Bi-directional Port

    lo = qc_handshake2 (q, 1);
    hi = (lo >> 8) & 0xff;
    lo = (lo >> 1) & 0x7f;

    lo2 = qc_handshake2 (q, 0);
    hi2 = (lo2 >> 8) & 0xff;
    lo2 = (lo2 >> 1) & 0x7f;

    hi ^= 0x80;					// This Bit is inverted by HW
    hi2 ^= 0x80;				//

    switch (q->color_mode) {
    case QC_COLOR_MILLION:			// Million Colors mode
      buffer[0] = ((hi  & 0x08) << 4) | lo;	// Red
      buffer[1] =  (hi  & 0xF0) | (hi2 >> 4);	// Green
      buffer[2] = ((hi2 & 0x08) << 4) | lo2;	// Blue

      return (3);

    case QC_COLOR_BILLION:			// Billion Colors mode 
      buffer[0] = ((hi  & 0x08) << 4) | lo;	// Red, Blue, Green2, Green1
      buffer[1] =  (hi  & 0xF0) | (hi2 >> 4);	// Green1, Red, Blue, Green2
      buffer[2] = ((hi2 & 0x08) << 4) | lo2;	// Green2, Green1, Red, Blue

      return (3);
    }

  case QC_UNIDIR:  				// Unidirectional Port
    hi = qc_handshake (q,1);
    lo = qc_handshake (q,0);

    lo >>= 4;

    hi &= 0xf0;					// mask byte
    lo &= 0x0f;					//

    hi ^= 0x80;					// invert HW inversion
    lo ^= 0x08;					//

    switch (q->color_mode) {
    case QC_COLOR_MILLION:
    case QC_COLOR_BILLION:
      buffer[0] = hi | lo;
      return (1);
    }
  }

  return (-1);
}


/******************************************************************/
/* Qc_readbytes reads some bytes from the QC and puts them in     */
/* the supplied buffer.  It returns the number of bytes read,     */
/* or -1 on error.                                                */

static int __inline__ qc_readbytes_bw (struct qcam *q, u_char buffer[])
{
  u_int hi, lo;
  u_int hi2, lo2;
  static u_int saved_bits;
  static int state = 0;

  switch (q->port_mode & QC_MODE_MASK) {
  case QC_BIDIR:  				// Bi-directional Port

    lo = qc_handshake2 (q, 1);
    hi = (lo >> 8) & 0xff;
    lo = (lo >> 1) & 0x7f;

    lo2 = qc_handshake2 (q, 0);
    hi2 = (lo2 >> 8) & 0xff;
    lo2 = (lo2 >> 1) & 0x7f;

    switch (q->bpp) {
      case 4:
        buffer[0] = lo & 0x0f;
        buffer[1] = ((lo & 0x70) >> 4) | (hi & 0x08);
        buffer[2] = (hi & 0xf0) >> 4;
        buffer[3] = (lo2 & 0xf0);
        buffer[4] = ((lo2 & 0x70) >> 4) | (hi2 & 0x08);
        buffer[5] = (hi2 & 0xf0) >> 4;
        return (6);

      case 6:
        buffer[0] = lo & 0x3f;
        buffer[1] = ((lo & 0x40) >> 6) | ((hi & 0xf8) >> 2);
        buffer[2] = lo2 & 0x3f;
        buffer[3] = ((lo2 & 0x40) >> 6) | ((hi2 & 0xf8) >> 2);
        return (4);
    }

  case QC_UNIDIR:  				// Unidirectional Port
    lo = (qc_handshake (q,1) & 0xf0) >> 4;
    hi = (qc_handshake (q,0) & 0xf0) >> 4;

    switch (q->bpp) {
    case 4:
      buffer[0] = lo;
      buffer[1] = hi;
      return (2);

    case 6:
      switch (state) {
      case 0:
        buffer[0] = (lo << 2) | ((hi & 0xc) >> 2);
        saved_bits = (hi & 0x03) << 4;
        state++;
        return (1);

      case 1:
        buffer[0] = saved_bits | lo;
        saved_bits = hi << 2;
        state++;
        return (1);

      case 2:
        buffer[0] = saved_bits | ((lo & 0xc) >> 2);
        buffer[1] = ((lo & 0x3) << 4) | hi;
        state = 0;
        return (2);
      }
    }
  }

  return (-1);
}
/******************************************************************/
/* Read a scan from the QC.  This takes the qcam structure and
 * requests a scan from the camera.  It sends the correct instructions
 * to the camera and then reads back the correct number of bytes.  In
 * previous versions of this routine the return structure contained
 * the raw output from the camera, and there was a 'qc_convertscan'
 * function that converted that to a useful format.  In version 0.3 I
 * rolled qc_convertscan into qc_scan and now I only return the
 * converted scan.  The format is just an one-dimensional array of
 * characters, one for each pixel, with 0=black up to n=white, where
 * n=2^(bit depth)-1.  Ask me for more details if you don't understand
 * this. */

scanbuf *qc_scan (struct qcam *q)
{
  u_char *ret=NULL;
  int i=0, j=0, k=0;
  int pixels;
  int lines, pixels_per_line;
  long pixels_read=0;
  u_char buffer[6];
//float red_gain = 1.5;

 //qc_getstatus (q);		// check if we are ready to get an image
				// ToDo: handle the result	


  qc_command (q, CMD_SendVideoFrame); qc_command (q, q->mode);

/**********/

  if ((q->port_mode & QC_MODE_MASK) == QC_BIDIR) {
    q->lp_control.bit.bidir = 1;
    qc_handshake (q, 1);
    qc_handshake (q, 0);
  }

/**********/

  lines = q->height / q->transfer_scale;
  pixels_per_line = q->width / q->transfer_scale;

  ret = malloc (lines * pixels_per_line * (int) (q->bpp/8. + 0.5));
  assert(ret);

#ifdef DEBUG
  qc_dump (q, NULL);
#endif

  if (qc_getversion (q) == BW_QUICKCAM) {
    for (i = 0; i < lines * pixels_per_line; i+=pixels) {
      pixels = qc_readbytes_bw (q, buffer);

      for (k = 0; k < pixels; k++) 
        ret [i + k] = (1 << q->bpp) - buffer[k] - 1;
    }
  }
  else if (qc_getversion (q) == COLOR_QUICKCAM) {
    if (q->color_mode == QC_COLOR_BILLION) {
      for (i=0; i < lines/2-1; i++) {
        for (j=0; j < pixels_per_line*2; j+=pixels) {
          pixels = qc_readbytes_color (q, buffer);

          for (k=0; k < pixels; k++) {
            switch (pixels_read%4) { 
              case 0:					// Red
		//buffer[k] = (buffer[k]*red_gain > 250) ? 250 : buffer[k]*red_gain;
		ret [(pixels_read/2+pixels_read/pixels_per_line/2*pixels_per_line)*3] = buffer [k];	//
		ret [(pixels_read/2+(pixels_read/pixels_per_line/2+1)*pixels_per_line)*3] = buffer [k];
		ret [(pixels_read/2+1+(pixels_read/pixels_per_line/2+1)*pixels_per_line)*3] = buffer [k];
		ret [(pixels_read/2+1+pixels_read/pixels_per_line/2*pixels_per_line)*3] = buffer [k];
                break;
              case 1:					// Green1
		ret [(pixels_read/2+pixels_read/pixels_per_line/2*pixels_per_line)*3+1] = buffer [k];
		ret [(pixels_read/2+(pixels_read/pixels_per_line/2+1)*pixels_per_line)*3+1] = buffer [k]; //
                break;
              case 2:					// Green2
		ret [(pixels_read/2+1+(pixels_read/pixels_per_line/2+1)*pixels_per_line)*3+1] = buffer [k]; //
		ret [(pixels_read/2+1+pixels_read/pixels_per_line/2*pixels_per_line)*3+1] = buffer [k];
                break;
              case 3:					// Blue
		ret [(pixels_read/2+pixels_read/pixels_per_line/2*pixels_per_line)*3+2] = buffer [k];
		ret [(pixels_read/2+(pixels_read/pixels_per_line/2+1)*pixels_per_line)*3+2] = buffer [k];
		ret [(pixels_read/2+1+(pixels_read/pixels_per_line/2+1)*pixels_per_line)*3+2] = buffer [k];
		ret [(pixels_read/2+1+pixels_read/pixels_per_line/2*pixels_per_line)*3+2] = buffer [k]; //
                break;
            }
            pixels_read++;
          }
        }
      }
    }
    else
    for (i = 0; i < lines * pixels_per_line * 3; i+=pixels) {
      pixels = qc_readbytes_color (q, buffer);

      for (k = 0; k < pixels; k++) {
        ret [i + k] = buffer [k];
      }
    }
    qc_geteof (q);				// read EOF
  }

						// Turn port around again.

/**********/

  if ((q->port_mode & QC_MODE_MASK) == QC_BIDIR) {
    q->lp_control.bit.bidir = 0;
    qc_handshake (q, 1);
    qc_handshake (q, 0);
  }

/**********/

  if (qc_getversion(q) == COLOR_QUICKCAM)
    qc_command (q, 0x00); 

  return (ret);
}

/******************************************************************/

void qc_geteof (struct qcam *q) 
{
  u_char buffer[6];
  int i, s, byte=0, save_byte=-1, ok=0;

  do {
    s = qc_readbytes_color (q, buffer);

    for (i=0; i<s; i++, byte++) {

#ifdef DEBUG
      //fprintf (stderr, "%x ", buffer[i]);
#endif

      if (buffer[i] == 0x0E) {
        ok = 1;
	save_byte = byte;
      }
      else if ((buffer[i] == 0x00) && (ok == 1)) {
        if (save_byte == byte-1) {
          ok = 2;
          save_byte = byte;
        }
        else
          ok = 0;
      }
      else if ((buffer[i] == 0x0F) && (ok == 2)) {
        if (save_byte == byte-1) 
          ok = 3;
        else
          ok = 0;
      }
    }
 } while (ok != 3);

#ifdef DEBUG
//      fprintf (stderr, "\n");
#endif
}

/******************************************************************/

static int qc_getportdir (struct qcam *q)
{
  int mode;

  switch (q->port_mode & QC_FORCE_MASK)
    {
    case QC_FORCE_UNIDIR:
      mode = (q->port_mode & ~QC_MODE_MASK) | QC_UNIDIR;
      break;

    case QC_FORCE_BIDIR:
      mode = (q->port_mode & ~QC_MODE_MASK) | QC_BIDIR;
      break;

    case QC_ANY:
      write_lpcontrol(q,0x20);
      write_lpdata(q,0x75);

      if (read_lpdata(q) != 0x75) {
	mode = (q->port_mode & ~QC_MODE_MASK) | QC_BIDIR;
      } else {
	mode = (q->port_mode & ~QC_MODE_MASK) | QC_UNIDIR;
      }
      break;

    case QC_FORCE_SERIAL:
    default:
      mode=0;
      fprintf(stderr, "Illegal port_mode %x\n", q->port_mode);
      break;
  }

  return (mode);
}

/******************************************************************/

void qc_dump (struct qcam *q, char *file)
{ FILE *fp;

  if (file == NULL)
    fp = stderr;
  else
    fp = fopen (file, "w+");

  fprintf (fp, "\n# Version %s\n", QC_DRV_VERSION);
  fprintf (fp, "Width: \t\t%d\tHeight: \t%d\n", q->width, q->height);   
  fprintf (fp, "Top \t\t%d\tLeft \t\t%d\n", q->top, q->left);   
  fprintf (fp, "Saturation \t%d\tContrast: \t%d\n", q->saturation, q->contrast);   
  fprintf (fp, "Bpp \t\t%d\tBrightness: \t%d\n", q->bpp, q->brightness);   
  fprintf (fp, "Blackbal: \t%d\tWhitebal: \t%d\n", q->blackbal, q->whitebal);   
  fprintf (fp, "Port \t\t0x%02x\tScale: \t\t%d\n", q->port, q->transfer_scale);   
  fprintf (fp, "Portmode \t%sDir\n", q->port_mode & QC_BIDIR ? "Bi" : "Uni");   

  if (file != NULL)
    fclose (fp);
}
