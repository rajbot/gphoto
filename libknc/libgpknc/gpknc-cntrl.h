#ifndef __GPKNC_CNTRL_H__
#define __GPKNC_CNTRL_H__

#include <libknc/knc-cntrl.h>
#include <gphoto2-port.h>

KncCntrl *gpknc_cntrl_new_from_port (GPPort *);
KncCntrl *gpknc_cntrl_new_from_path (const char *);

#endif
