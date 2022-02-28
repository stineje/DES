# Makefile

CC     = gcc
CFLAGS = -O3
LIBS   = -lm
SRCS   = $(wildcard *.c)

PROGS = $(patsubst %.c,%,$(SRCS))

all:	$(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean: 
	rm -f $(PROGS)
	rm -f *~


