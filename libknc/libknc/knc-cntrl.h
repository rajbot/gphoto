#ifndef __KNC_CNTRL_H__
#define __KNC_CNTRL_H__

#include <libknc/knc-cntrl-res.h>
#include <libknc/knc-cam-res.h>

#include <stdarg.h>

typedef enum {
	KNC_CNTRL_PROT_NONE	= 0,
	KNC_CNTRL_PROT_LONG_ID	= 1 << 1
} KncCntrlProt;

typedef struct _KncCntrl KncCntrl;

typedef KncCntrlRes (* KncCntrlFuncRead)  (unsigned char *, unsigned int,
					   unsigned int, unsigned int *,
					   void *);
typedef KncCntrlRes (* KncCntrlFuncWrite) (const unsigned char *,
					   unsigned int, void *);

/* Life-cycle */
KncCntrl *knc_cntrl_new   (KncCntrlFuncRead, KncCntrlFuncWrite, void *);
void      knc_cntrl_ref   (KncCntrl *);
void      knc_cntrl_unref (KncCntrl *);
typedef void (* KncCntrlFuncFree) (KncCntrl *, void *);
void      knc_cntrl_set_func_free (KncCntrl *, KncCntrlFuncFree  , void  *);
void      knc_cntrl_get_func_free (KncCntrl *, KncCntrlFuncFree *, void **);

/* Transmitting data */
typedef KncCntrlRes (* KncCntrlFuncData)  (const unsigned char *,
					   unsigned int, void *);
void knc_cntrl_set_func_data (KncCntrl *, KncCntrlFuncData, void *);
KncCntrlRes knc_cntrl_transmit   (KncCntrl *,
				  const unsigned char *, unsigned int,
				        unsigned char *, unsigned int *);

/* Querying the protocol used */
KncCntrlProt knc_cntrl_prot       (KncCntrl *c);

/* For debugging */
typedef void (* KncCntrlFuncLog) (const char *format, va_list, void *);
void knc_cntrl_set_func_log (KncCntrl *c, KncCntrlFuncLog, void *);

/* For testing */
KncCntrlRes knc_cntrl_test (KncCntrl *c);

#endif
