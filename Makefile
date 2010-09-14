CC=gcc

OBJS=main.o sexp.o print.o read.o

PROGRAM=object

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -rf *.o $(PROGRAM)