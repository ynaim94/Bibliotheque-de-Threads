CFLAGS=-Wall 
LDFLAGS=-g
CC=gcc
THREAD= -DUSE_PTHREAD -lpthread

EXEC=01-main 51-fibonacci 02-switch 61-mutex 11-join 12-join-main 21-create-many 22-create-many-recursive 23-create-many-once 31-switch-many 32-switch-many-join

EXEC-PTHREAD=$(addsuffix -pthread, $(EXEC))

OBJ=$(addsuffix .o, $(EXEC) $(EXEC-PTHREAD))

check: all

all: preemption_exemple thread.o $(EXEC) $(EXEC-PTHREAD)

example: example.o thread.o	
	$(CC) -o $@ $^ $(LDFLAGS)
01-main: 01-main.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
02-switch: 02-switch.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
11-join: 11-join.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
12-join-main: 12-join-main.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
21-create-many: 21-create-many.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
22-create-many-recursive: 22-create-many-recursive.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
23-create-many-once: 23-create-many-once.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
31-switch-many: 31-switch-many.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
32-switch-many-join: 32-switch-many-join.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
51-fibonacci: 51-fibonacci.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
61-mutex: 61-mutex.o thread.o
	$(CC) -o $@ $^ $(LDFLAGS)
preemption_exemple: preemption_exemple.o thread_preemption_pseudo.o
	$(CC) -o $@ $^ $(LDFLAGS)

%-pthread: %.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(THREAD)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) $(EXEC) *-pthread *.o *~ *#

.PHONY: clean all


