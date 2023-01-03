CC = gcc
FLAGS = -Wall -g

.PHONY: all clean

all: ping better_ping watchdog

ping: ping.o
	$(CC) $(FLAGS) -o PartA ping.o

better_ping: better_ping.o
	$(CC) $(FLAGS) -o PartB better_ping.o

watchdog: watchdog.o
	$(CC) $(FLAGS) -o watchdog watchdog.o

ping.o: ping.c
	$(CC) $(FLAGS) -c ping.c

better_ping.o: better_ping.c
	$(CC) $(FLAGS) -c better_ping.c

watchdog.o: watchdog.c
	$(CC) $(FLAGS) -c watchdog.c

clean:
	rm -f *.o PartA PartB watchdog
