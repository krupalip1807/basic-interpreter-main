CC=gcc
CFLAGS=-Wall -ggdb -DINTERPRETER
OBJS=lex.o stack.o expr.o run.o lang.o as.c
DEST=lang
LDFLAGS=
$(DEST): $(OBJS)
	$(CC) $(CFLAGS) -o $(DEST) $(OBJS) $(LDFLAGS)

$(OBJS): lang.h

clean:
	rm -f *.o $(DEST)
