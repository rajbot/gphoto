all: runtime dynamic unified math

runtime: library runtime.c
	gcc -rdynamic -g -o runtime runtime.c -ldl

dynamic: library dynamic.c
	gcc -rdynamic -g -o dynamic dynamic.c -ldl -L./ -lcamera

library: library.c
	gcc -c library.c -fPIC
	gcc -shared -Wl,-soname,libcamera.so -o libcamera.so library.o

unified: unified.c
	gcc -g -o unified unified.c

math: math.c
	 gcc -g -rdynamic -o math math.c -ldl

clean:
	rm -f *.o *~ core math unified dynamic runtime libcamera.so
