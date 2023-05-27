CC = gcc
FLAGS = -Wall -g

.PHONY: all clean

all: ping new_ping watchdog

ping: ping.o
	$(CC) $(FLAGS) -o PartA ping.o

new_ping: new_ping.o
	$(CC) $(FLAGS) -o PartB new_ping.o

watchdog: watchdog.o
	$(CC) $(FLAGS) -o watchdog watchdog.o

ping.o: ping.c
	$(CC) $(FLAGS) -c ping.c

new_ping.o: new_ping.c
	$(CC) $(FLAGS) -c new_ping.c

watchdog.o: watchdog.c
	$(CC) $(FLAGS) -c watchdog.c

clean:
	rm -f *.o PartA PartB watchdog
