/* qcam.h -- routines for accessing the Connectix QuickCam */

/* Version 0.1, January 2, 1996 */
/* Version 0.5, August 24, 1996 */
/* Version 0.7, August 26, 1996 */

#define QC_DRV_VERSION "0.9pre6"

/******************************************************************

Copyright (C) 1996 by Scott Laird

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

/* One from column A... */
#ifndef __QCAM__

#define __QCAM__

#define MAX_HEIGHT 320
#define MAX_WIDTH 240

#define QC_TIMEOUT 500

#define QC_NOTSET 0
#define QC_UNIDIR 1
#define QC_BIDIR  2
#define QC_SERIAL 3

/* ... and one from column B */
#define QC_ANY          0x00
#define QC_FORCE_UNIDIR 0x10
#define QC_FORCE_BIDIR  0x20
#define QC_FORCE_SERIAL 0x30
/* in the port_mode member */

#define QC_MODE_MASK    0x07
#define QC_FORCE_MASK   0x70

/* ... the Color Modes for the ColorCam*/
#define QC_COLOR_THOUSAND 555
#define QC_COLOR_MILLION 888
#define QC_COLOR_BILLION 8888

/* ... the Quickcam commands */
#define CMD_SendVideoFrame 0x07
#define CMD_SetBrightness  0x0B
#define CMD_SetTop         0x0D
#define CMD_SetLeft        0x0F
#define CMD_SetNumV        0x11
#define CMD_SetNumH        0x13
#define CMD_SendVersion    0x17

/* ... the BW specific commands */
#define CMD_BW_SetContrast      0x19
#define CMD_BW_AutoAdjustOffset 0x1B
#define CMD_BW_BytePortEcho     0x1D
#define CMD_BW_SetOffset        0x1F
#define CMD_BW_GetOffset        0x21

/* ... the COLOR specific commands */
#define CMD_COLOR_LoadRAM       0x1B
#define CMD_COLOR_SetBlack      0x1D
#define CMD_COLOR_SetWhite      0x1F
#define CMD_COLOR_SetHue        0x21
#define CMD_COLOR_SetSaturation 0x23
#define CMD_COLOR_SetContrast   0x25
#define CMD_COLOR_SendStatus    0x29
#define CMD_COLOR_SetSpeed      0x2D

/* definition for qcam versions */
#define BW_QUICKCAM    0x01
#define COLOR_QUICKCAM 0x10

#define LP_STROBE		0x01
#define LP_AUTOFEED		0x02
#define LP_RESET_N		0x04
#define LP_PCACK		0x08
#define LP_BIDIR		0x20

union qcam_lp_control {
	struct {
	  char strobe:1;
	  char autofeed:1;
	  char reset_n:1;
	  char pcack:1;
	  char junk4:1;
	  char bidir:1;
	  char junk6:1;
	  char junk7:1;
	} bit;

	unsigned char data;
};


struct qcam {
  int width, height;
  int bpp;
  int color_mode;
  int mode;
  int contrast, brightness, whitebal, blackbal, hue, saturation;
  int port;
  int port_mode;
  int transfer_scale;
  int top, left;
  int cam_version;
  int speed;
  int fd; /* lock file descriptor
           * It was, unfortunately, necessary to add this member to the
           * struct qcam to conveniently implement POSIX fcntl-style locking.
           * We need a seperate lock file for each struct qcam, for instance,
           * if the same process (using qcam-lib) is accessing multiple
           * QuickCams on (of course) multiple ports.
           * - Dave Plonka (plonka@carroll1.cc.edu)
           */
  union qcam_lp_control lp_control;
  int lp_control_data;
};

typedef unsigned char scanbuf;

/* General QuickCam handling routines */

void qc_dump (struct qcam *q, char *file);
int qc_getbrightness(struct qcam *q);
int qc_setbrightness(struct qcam *q, int val);
int qc_getcontrast(struct qcam *q);
int qc_setcontrast(struct qcam *q, int val);
int qc_getwhitebal(struct qcam *q);
int qc_setwhitebal(struct qcam *q, int val);
int qc_setblackbal(struct qcam *q, int val);
void qc_getresolution(struct qcam *q, int *x, int *y);
int qc_setresolution(struct qcam *q, int x, int y);
int qc_getbpp(struct qcam *q);
int qc_setbpp(struct qcam *q, int val);
int qc_getheight(struct qcam *q);
int qc_setheight(struct qcam *q, int y);
int qc_getwidth(struct qcam *q);
int qc_setwidth(struct qcam *q, int x);
int qc_gettop(struct qcam *q);
int qc_settop(struct qcam *q, int val);
int qc_getleft(struct qcam *q);
int qc_setleft(struct qcam *q, int val);
int qc_gettransfer_scale(struct qcam *q);
int qc_settransfer_scale(struct qcam *q, int val);
int qc_calibrate(struct qcam *q);
int qc_forceunidir(struct qcam *q);

struct qcam *qc_init(void);
int qc_initfile(struct qcam *q, char *fname);
int qc_open(struct qcam *q);
int qc_close(struct qcam *q);
int qc_detect(struct qcam *q);
void qc_reset(struct qcam *q);
void qc_set(struct qcam *q);
scanbuf *qc_scan(struct qcam *q);
scanbuf *qc_convertscan(struct qcam *q, scanbuf *scan);

int qc_getversion (struct qcam *q);
int qc_setport (struct qcam *q, int val);
int qc_getport (struct qcam *q);
int qc_setsaturation (struct qcam *q, int val);
int qc_getsaturation (struct qcam *q);
int qc_sethue (struct qcam *q, int val);
int qc_gethue (struct qcam *q);

/* OS/hardware specific routines */

int read_lpstatus	(const struct qcam *q);
int read_lpstatus_data	(const struct qcam *q);
int read_lpcontrol	(const struct qcam *q);
int read_lpdata		(const struct qcam *q);
void write_lpdata	(const struct qcam *q, int d);
void write_lpcontrol	(const struct qcam *q, int d);
int enable_ports	(struct qcam *q);
int disable_ports	(struct qcam *q);
int qc_unlock		(struct qcam *q);
int qc_lock		(struct qcam *q);
void qc_wait		(int val);
int qc_probe		(struct qcam *q);

#endif
