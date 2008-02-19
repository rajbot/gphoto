CC = gcc
CFLAGS = -g -Wall -O3 `pkg-config --cflags --libs gtk+-2.0`
OBJS = main.o mr_process.o byr2ppm.o
EXE = show_mr_raw

all: $(EXE)

$(EXE): $(OBJS)
	gcc  $(CFLAGS) $(OBJS) -o $(EXE)
	
# rule to build $(OBJS) is implicit

clean:
	rm $(EXE) *.o
