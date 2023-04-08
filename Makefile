CC=gcc
CFLAGS:= -lm -Ofast -mavx2 -Wall -m64 -s -funsafe-math-optimizations -fno-finite-math-only
#CFLAGS:= -lm -Wall -m64 -fsanitize=address -g
#CFLAGS:= -lm -Wall -m64 -O3 -ffinite-math-only
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
	rm -rf $(obj) Font.font.h

Font.font.h: Font.bmp
	python3 NoFont/img2font.py Font.bmp "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890()[]{}!?\"=/\'+-*#.,_" true

dft.o: Font.font.h

test: dft
	./dft monk.stl FROM MESH TO NBT AS nbt.nbt
	./dft nbt.nbt FROM NBT TO MESH AS out.obj
