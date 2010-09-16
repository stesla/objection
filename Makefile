CC=gcc

OBJS=main.o alloc.o object.o print.o read.o error.o buffer.o

CFLAGS=-g

PROGRAM=object
all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -rf *.o $(PROGRAM)