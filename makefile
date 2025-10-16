CC = gcc
CFLAGS = -Wall -pthread

all: bounded_buffer mh airline

bounded_buffer: bounded_buffer.c
	$(CC) $(CFLAGS) -o bounded_buffer bounded_buffer.c

mh: mother_hubbard.c
	$(CC) $(CFLAGS) -o mh mother_hubbard.c

airline: airline.c
	$(CC) $(CFLAGS) -o airline airline.c

clean:
	rm -f bounded_buffer mh airline