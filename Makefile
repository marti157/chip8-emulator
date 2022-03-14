CC = g++
CFLAGS = -std=c++17 -Wall

ifeq ($(shell sh -c 'uname 2>/dev/null'), Darwin) # Mac OS
	$(error Use XCode on Mac OS)
endif

chip8: main.o chip8.o
	$(CC) $(CFLAGS) -o chip8 main.o chip8.o `sdl2-config --cflags --libs`

main.o: main.cpp chip8.h
	$(CC) $(CFLAGS) -c main.cpp

chip8.o: chip8.cpp chip8.h
	$(CC) $(CFLAGS) -c chip8.cpp

clean:
	rm -f chip8 main.o chip8.o
