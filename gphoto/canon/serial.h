/****************************************************************************
 *
 * File: serial.h
 *
 ****************************************************************************/

#ifndef _SERIAL_H
#define _SERIAL_H

/****************************************************************************
 *
 * prototypes
 *
 ****************************************************************************/

int canon_serial_init(const char *devname);
int canon_serial_restore();
int canon_serial_send(const unsigned char *buf, int len);
int canon_serial_get_byte();

#endif /* _SERIAL_H */

/****************************************************************************
 *
 * End of file: serial.h
 *
 ****************************************************************************/
