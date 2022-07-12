CC=clang
CFLAGS:= -lm -Ofast -mavx2 -Wall -m64 -s -funsafe-math-optimizations
source:= $(wildcard *.c) \
				 $(wildcard List/*.c)

obj:= $(source:.c=.o)

dft: $(obj)
	$(CC) $^ $(CFLAGS) -o $@
clean:
	rm $(obj)
