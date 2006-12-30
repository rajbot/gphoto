#include <dlfcn.h>

int main(int argc, char **argv) {
 void *handle = dlopen ("/usr/lib/libm.so", RTLD_LAZY);
 double (*cosine)(double) = dlsym(handle, "cos");
 printf ("%f\n", (*cosine)(2.0));
 dlclose(handle);
}

