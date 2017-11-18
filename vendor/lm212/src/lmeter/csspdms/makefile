#
# Makefile for the csspdms.a library
#

CC = cc
MAKE = make
CPP = $(CC) -M 

CFLAGS = -g -O3 -finline-functions 

.c.o:
	$(CC) $(CFLAGS) -c $<

OBJS=	directso.o  iterativ.o  numbfact.o  spmatrix.o  symbfact.o	\
	factoriz.o  lowlevel.o  permute.o   spvector.o

all: csspdms.a

csspdms.a: $(OBJS) 
	ar r csspdms.a $(OBJS)

clean:
	rm -f core *.s *.o *.a *~ *%

depend dep:
	$(CPP) $(CFLAGS) -M *.c > .depend
#
# include a dependency file if one exists
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif





