#ifndef KODAK_GENERIC_H

#define KODAK_GENERIC_H

#include <gdk_imlib.h>
#include <gdk/gdk.h>

#include "../src/gphoto.h"
#include "io.h"

int 		kodak_generic_initialize();
GdkImlibImage *	kodak_generic_get_picture(int, int);
GdkImlibImage * kodak_generic_get_preview();
int		kodak_generic_delete_picture(int);
int		kodak_generic_take_picture();
int		kodak_generic_number_of_pictures();
int		kodak_generic_configure();
char *		kodak_generic_summary();
char *		kodak_generic_description();

struct _Camera kodak_generic = {kodak_generic_initialize,
                              kodak_generic_get_picture,
                              kodak_generic_get_preview,
	                      kodak_generic_delete_picture,
	                      kodak_generic_take_picture,
	                      kodak_generic_number_of_pictures,
	                      kodak_generic_configure,
	                      kodak_generic_summary,
	                      kodak_generic_description};

#endif
