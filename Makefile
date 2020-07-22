EXECS=exec
CC = gcc -std=gnu99 -ansi -pedantic
CFLAGS = -g -Wall -fno-inline -O3
SRCS = arith.c except.c assert.c memchk.c stack.c queue.c list.c atom.c
OBJS = $(SRCS:.c=.o)
MAIN=exec
LIBFLAGS= -shared -Wl -soname
AR=ar rcs

.PHONY: depend clean

all: LIB
	@echo  Library has been compiled

$(MAIN): $(OBJS) main.c
	$(CC) $(CFLAGS) main.c -o $(MAIN) $(OBJS)

LIB: $(OBJS)
	$(AR) libmyclib.a $(OBJS)

.c.o:
	$(CC) -fPIC $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) *.o *~ $(MAIN) *.a

depend: $(SRCS)
	makedepend $(INCLUDES) $^

#DO NOT DELETE
