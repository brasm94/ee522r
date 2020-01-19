CC=gcc
CFLAGS=-I.
OBJ = server.o
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
.PHONY: clean

clean:
	rm *.o server

run: server
	./server 8080

all:clean run
	