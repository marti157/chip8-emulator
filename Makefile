CC = g++
CFLAGS = -Wall -lSDL2
DEPS = chip8.h
OBJS = main.o chip8.o
OUT = chip8

ifeq ($(shell sh -c 'uname 2>/dev/null'), Darwin) # Mac OS
	$(error Use XCode on Mac OS)
endif

%.o: %.cc $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

chip8: $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -f chip8
