#include <stdio.h>
#include <dlfcn.h>

typedef void (*_register_call) (int);

void register_call(int i);

void register_call(int i)
{
	printf("%d\n",i);
}

int main()
{
	void *handle = dlopen ("/home/phill/src/GPhoto/dynamic/examples/libcamera.so", RTLD_LAZY);

	void (*remote_function)(_register_call, int) = dlsym(handle, "remote");

	_register_call register_function = &register_call;

	remote_function(register_function, 12345);

	dlclose(handle);
}
