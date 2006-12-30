#include <stdio.h>

extern reg_func;

typedef void (*_register_call) (int);

void remote(_register_call reg_func, int i);

void remote(_register_call reg_func, int i)
{
	(reg_func)(i);
}
