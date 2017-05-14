CFLAGS=-Wall 
LDFLAGS=-g
CC=gcc
THREAD= -DUSE_PTHREAD -lpthread

EXEC=01-main 51-fibonacci 02-switch 61-mutex 11-join 12-join-main 21-create-many 22-create-many-recursive 23-create-many-once 31-switch-many 32-switch-many-join

EXEC-PTHREAD=$(addsuffix -pthread, $(EXEC))

TEST=$(addprefix /test/, $(EXEC) $(EXEC-PTHREAD))

OBJ=$(addsuffix .o, $(TEST))

check: all

all: $(EXEC) $(EXEC-PTHREAD) preemption_exemple

01-main: test/01-main.c src/thread.c
	$(CC) -o build/$@ $^ $(LDFLAGS)

02-switch: test/02-switch.c src/thread.c
	$(CC) -o build/$@ $^ $(LDFLAGS)

11-join: test/11-join.c src/thread.c
	$(CC) -o build/$@ $^ $(LDFLAGS)

12-join-main: test/12-join-main.c src/thread.c
	$(CC) -o build/$@ $^ $(LDFLAGS)

21-create-many: test/21-create-many.c src/thread.c
	$(CC) -o build/$@ $^ $(LDFLAGS)

22-create-many-recursive: test/22-create-many-recursive.c src/thread.c
	$(CC) -o build/$@ $^ $(LDFLAGS)

23-create-many-once: test/23-create-many-once.c src/thread.c
	$(CC) -o build/$@ $^ $(LDFLAGS)

31-switch-many: test/31-switch-many.c src/thread.c
	$(CC) -o build/$@ $^ $(LDFLAGS)

32-switch-many-join: test/32-switch-many-join.c src/thread.c
	$(CC) -o build/$@ $^ $(LDFLAGS)

51-fibonacci: test/51-fibonacci.c src/thread.c
	$(CC) -o build/$@ $^ $(LDFLAGS)

61-mutex: test/61-mutex.c src/thread.c
	$(CC) -o build/$@ $^ $(LDFLAGS)


preemption_exemple: test/preemption_exemple.c src/thread_preemption_pseudo.c
	$(CC) -o build/$@ $^ $(LDFLAGS)


%-pthread: test/%.c
	$(CC) $^ -o build/$@ $(CFLAGS) $(LDFLAGS) $(THREAD)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) build/*

.PHONY: clean all


