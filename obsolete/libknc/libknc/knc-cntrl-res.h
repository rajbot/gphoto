#ifndef __KNC_CNTRL_RES_H__
#define __KNC_CNTRL_RES_H__

typedef enum {
	KNC_CNTRL_OK = 0,
	KNC_CNTRL_ERR,
	KNC_CNTRL_ERR_CANCEL,
	KNC_CNTRL_ERR_NO_MEMORY,
	KNC_CNTRL_ERR_ILLEGAL_PARAMETER,
} KncCntrlRes;

const char *knc_cntrl_res_name (KncCntrlRes);

#endif
