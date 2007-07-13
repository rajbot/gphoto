#include "config.h"

#include <stdio.h>
#include <gettext.h>

#ifdef ENABLE_NLS
# define _(msgid) gettext(msgid)
#else
# define _(msgid) msgid
#endif

int main(int argc, char *argv[])
{
	printf("i18n-test\n");
	printf("%s\n", _("Erk"));
	return 0;
}
