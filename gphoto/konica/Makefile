CFLAGS = -g -Wall -I/usr/lib/glib/include

OBJS = \
	qm100.o \
	getPicInfo.o \
	getStatus.o \
	close.o \
	savePic.o \
	saveThumb.o \
	erasePic.o \
	takePic.o \
	error.o \
	setSpeed.o \
	open.o \
	transmission.o \
	lowlevel.o \
	formatCF.o

PLUG_OBJS = \
	konica_qm100.o \
	getPicInfo.o \
	transmission.o \
	lowlevel.o \
	getStatus.o \
	close.o \
	savePic.o \
	saveThumb.o \
	erasePic.o \
	formatCF.o \
	takePic.o \
	error.o \
	setSpeed.o \
	open.o \
	../src/util.o

.c.o:	
	gcc ${CFLAGS} -c -fPIC $<

libgphoto_konica_qm100.so: ${PLUG_OBJS}
	gcc -shared -Wl,-soname,libgphoto_konica_qm100.so -o libgphoto_konica_qm100.so ${PLUG_OBJS}

konica_qm100.a: ${PLUG_OBJS}
	ar -rc konica_qm100.a ${PLUG_OBJS}

qm100:
	gcc ${CFLAGS} -D_CLI_ -c qm100.c
	gcc ${CFLAGS} -D_CLI_ -c getPicInfo.c
	gcc ${CFLAGS} -D_CLI_ -c getStatus.c
	gcc ${CFLAGS} -D_CLI_ -c close.c
	gcc ${CFLAGS} -D_CLI_ -c savePic.c
	gcc ${CFLAGS} -D_CLI_ -c saveThumb.c
	gcc ${CFLAGS} -D_CLI_ -c erasePic.c
	gcc ${CFLAGS} -D_CLI_ -c takePic.c
	gcc ${CFLAGS} -D_CLI_ -c error.c
	gcc ${CFLAGS} -D_CLI_ -c setSpeed.c
	gcc ${CFLAGS} -D_CLI_ -c open.c
	gcc ${CFLAGS} -D_CLI_ -c formatCF.c
	gcc ${CFLAGS} -D_CLI_ -c transmission.c
	gcc ${CFLAGS} -D_CLI_ -c lowlevel.c
	gcc ${CFLAGS} -D_CLI_ -c getStatus.c
	gcc -o $@ ${OBJS}

clean:
	rm -f *~ *.o qm100 core konica_qm100.a libgphoto_konica_qm100.so

