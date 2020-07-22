EXECS=exec
CC = gcc -std=gnu99 -ansi -pedantic
CFLAGS = -g -Wall -fno-inline -O2
SRCS = arith.c except.c assert.c memchk.c stack.c queue.c list.c main.c atom.c
#/home/grads/vardas/C_INTERFACES
#INCLUDES = -I/home/john/WoRK/C_INTERFACES
OBJS = $(SRCS:.c=.o)
MAIN=exec
LIBFLAGS= -shared -Wl -soname

.PHONY: depend clean

all: $(MAIN)
	@echo  Library has been compiled

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS)

LIB: $(OBJS)
	$(CC) $(CFLAGS) -shared -o libmyclib.so $(OBJS)

.c.o:
	$(CC) -fPIC $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) *.o *~ $(MAIN) *.so GTAGS GRTAGS GPATH

depend: $(SRCS)
	makedepend $(INCLUDES) $^

#DO NOT DELETE
