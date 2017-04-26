CFLAGS=-Wall 
LDFLAGS=-g
CC=gcc

SRC= example.c 01-main.c 51-fibonacci.c 02-switch.c 61-mutex.c 11-join.c 12-join-main.c thread.c 21-create-many.c 22-create-many-recursive.c 23-create-many-once.c 31-switch-many.c 32-switch-many-join.c      

OBJ=$(SRC:.c=.o)

BIN=$(SRC:.c= )

check: all

all: $(BIN)

example-pre: example.o thread.o
	$(CC) -E example.c

example: example.o thread.o	
	$(CC) -o $@ $^ $(LDFLAGS)
main: 01-main.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
switch: 02-switch.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
join: 11-join.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
join-main: 12-join-main.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
create-many: 21-create-many.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
create-many-recursive: 22-create-many-recursive.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
create-many-once: 23-create-many-once.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
switch-many: 31-switch-many.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
switch-many-join: 32-switch-many-join.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
fibonacci: 51-fibonacci.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
mutex: 61-mutex.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
thread: thread.o
	$(CC) -o $@ $^ $(LDFLAGS)

thread.o : thread.h

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(LDFLAGS)


clean:
	$(RM) $(OBJ) $(BIN) *.ppm *~ *#

.PHONY: clean all


