CC=gcc
SRCS=main.c pagerank.c graph.c thr_pool.c
OBJS=$(SRCS:%.c=./bin/%.o)

all: pre-build $(OBJS)
	$(CC) $(OBJS) -o ./bin/main

pre-build:
	mkdir -p ./bin

./bin/%.o: ./%.c
	$(CC) -c $< -o $@

clean:
	rm -rf ./bin