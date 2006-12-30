#include <stdio.h>
#include <dlfcn.h>

typedef void (*_register_call) (int);

void register_call(int i);

extern void remote(_register_call, int);

void register_call(int i)
{
	printf("%d\n",i);
}

int main()
{
	_register_call register_function = &register_call;

	remote(register_function, 12345);
}
