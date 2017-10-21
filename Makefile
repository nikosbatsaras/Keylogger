CC = gcc
CFLAGS = -g -Wall
OBJECTS = main.o keylogger.o

.PHONY: all clean

all: $(OBJECTS)
	$(CC) $(CFLAGS) -o keylogger $(OBJECTS)

main.o: main.c keylogger.o
	$(CC) $(CFLAGS) -c main.c

keylogger.o: keylogger.c keylogger.h
	$(CC) $(CFLAGS) -c keylogger.c

clean:
	@rm *.o keylogger 2> /dev/null || true
