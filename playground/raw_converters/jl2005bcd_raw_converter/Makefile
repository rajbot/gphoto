CC = gcc
LD=gcc
CFLAGS = -lm -ljpeg
OBJS = decompress.o
EXE = jl2005bcd_decompress


all: $(EXE)

$(EXE): $(OBJS)
	gcc  $(CFLAGS) $(OBJS) -o $(EXE)
	
# rule to build $(OBJS) is implicit

clean:
	rm $(EXE) *.o
