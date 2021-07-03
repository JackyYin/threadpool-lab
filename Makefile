CC=gcc
INCLUDE_DIR=src

LIBS= -lpthread \
	  -lm

test: threadpool.o queue.o
	$(CC) -g -o $@ $^ test.c -I$(INCLUDE_DIR) $(LIBS)

atomic_test: atomic_threadpool.o atomic_queue.o
	$(CC) -o $@ $^ atomic_test.c -I$(INCLUDE_DIR) $(LIBS)

%.o: src/%.c
	$(CC) -o $@ -c $<

clean:
	-@rm -f test atomic_test *.o
