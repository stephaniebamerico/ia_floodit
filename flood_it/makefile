CC = gcc
CFLAGS = -Wall -g

all: flood_it

clean:
	-rm -f flood_it graph

graph: graph.h graph.c
	$(CC) $(CFLAGS) -o graph -c graph.c

flood_it: graph flood_it.h flood_it.c
	$(CC) $(CFLAGS) -o flood_it flood_it.c graph