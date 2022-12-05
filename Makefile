CC=gcc
#CFLAGS:= -lm -Ofast -mavx2 -Wall -m64 -s -funsafe-math-optimizations
CFLAGS:= -lm -Wall -m64 -fsanitize=address -g
source:= $(wildcard *.c) \
				 $(wildcard List/*.c) \
				 $(wildcard info/*.c) \
				 $(wildcard NoFont/*.c) \
				 $(wildcard info/List/*.c) \
				 $(wildcard zlib/*.c)

obj:= $(source:.c=.o)
dft: $(obj) 
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm $(obj)

Font.font.h: Font.bmp
	python NoFont/img2font.py Font.bmp "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890()[]{}!?\"=/\'+-*#.,_" true

dft.o: Font.font.h

