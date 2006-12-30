#ifndef __KNC_UTILS_H__
#define __KNC_UTILS_H__

void knc_escape   (char *, unsigned int *);
void knc_unescape (char *, unsigned int *);

unsigned int knc_count_devices           (void);
const char  *knc_get_device_manufacturer (unsigned int);
const char  *knc_get_device_model        (unsigned int);

#endif
