# Just in case someone wants to print this...

GENERATED = REQUIREMENTS.ps

.PHONY: all clean
all: $(GENERATED)

clean:
	rm -f $(GENERATED)

PAPER = A4

REQUIREMENTS.ps: REQUIREMENTS
	iconv -f utf-8 -t iso-8859-1 "$<" | a2ps -M"$(PAPER)" -o"$@" -t"$<" --stdin="gphoto project3/REQUIREMENTS" -
