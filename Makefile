keylogger: keylogger.c main.c
	gcc -Wall -c keylogger.c
	gcc -Wall -o keylogger main.c keylogger.o

clean:
	rm *.o keylogger
