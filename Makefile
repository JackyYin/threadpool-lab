CC=gcc
INCLUDE_DIR=src

LIBS= -lpthread \
	  -lm

test: threadpool.o queue.o
	$(CC) -g -o $@ $^ test.c -I$(INCLUDE_DIR) $(LIBS)

%.o: src/%.c
	$(CC) -o $@ -c $<

clean:
	-@rm -f test atomic_test *.o
