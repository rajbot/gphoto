#include <stdio.h>

typedef void (*_register_call) (int);


int main(int argc, char **argv);
void register_call(int i);
void remote(_register_call reg_func, int i);

void register_call(int i)
{
	printf("%d\n",i);
}

int main(int argc, char **argv)
{
	_register_call x = &register_call;

	remote(x, 12345);
}

void remote(_register_call reg_func, int i)
{
	(reg_func)(i);
}



