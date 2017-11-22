#
# Makefile for the LMeter
#

CC=gcc
YACC=bison
LEX=flex

CFLAGS = -O -g 

.c.s:
	$(CC) $(CFLAGS) -S $<
.c.o:
	$(CC) $(CFLAGS) -c $<
.s.o:
	$(AS) -o $*.o $<

OBJS=	cif.tab.o cifgeom.o  lex.yy.o list.o

all: cif2dxc 

cif2dxc: $(OBJS) cif2dxc.o
	$(CC) -o cif2dxc cif2dxc.o $(OBJS) -lm

cif2ps: $(OBJS) cif2ps.o 
	$(CC) -o cif2ps cif2ps.o $(OBJS) -lm

lex.yy.c: cif.l 
	$(LEX) $<

cif.tab.c: cif.y 
	$(YACC) -d $<

clean:
	-rm -f core *.s *.o *.a *~ *% *.tab.* *.yy.* cif2dxc

install: cif2dxc
	cp -f cif2dxc ../../bin

depend dep:
	$(CPP) $(CFLAGS) -M *.c > .depend
#
# include a dependency file if one exists
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif
