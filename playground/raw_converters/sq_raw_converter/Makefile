CC = gcc
#LD=gcc
#CFLAGS = -g -Wall --pedantic -O3
CFLAGS = -g -Wall -O3 `pkg-config --cflags --libs gtk+-2.0`

#LDFLAGS = -lm;
OBJS = main.o sq_process.o byr2ppm.o
EXE = show_sq_raw


all: $(EXE)

$(EXE): $(OBJS)
	gcc  $(CFLAGS) $(OBJS) -o $(EXE)
	
# rule to build $(OBJS) is implicit

clean:
	rm $(EXE) *.o
