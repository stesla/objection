CC=gcc

OBJS=main.o alloc.o object.o print.o read.o error.o buffer.o env.o \
	builtins.o gc.o eval.o

CFLAGS=-g -Wall

PROGRAM=object
all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -rf *.o $(PROGRAM)

test: $(PROGRAM)
	./test.sh