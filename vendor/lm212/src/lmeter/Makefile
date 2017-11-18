#
# Makefile for the LMeter
#

SHELL = /bin/csh

CC = gcc
MAKE = make
CPP = $(CC) -M 

#	The following -Defines may be added here:
#       -DOLDLM3 to get .dat files for the old solver (not supported now).
#       -DNOINDCORR to suppress boundary inductance correction.
#       -DNDEBUG when completely tested :-) ...
#	-DPSGRAPH to produce a nice picture in PostScript format :)
#	-DLONGCNF to use long long for config vectiors, allowing 64 terminals
#	-DPTHREADS to use multithreading in solver. Usable with Linux Threads.

CFLAGS = -DLONGCNF -Icsspdms -g -O3 \
	-finline-functions -funroll-loops -ffast-math \
	-DPSGRAPH
# -DPTHREADS -DNUMTHREADS=2 
# -g -O3 -DPSGRAPH -finline-functions
# +O3 -Aa
#-O2 -mips2  

LDFLAGS = -static

MALLOCDEBUG = 
#-lefence

# THREADLIB = -lpthread

MAKEDEFS = CC='$(CC)' CPP='$(CPP)' CFLAGS='$(CFLAGS)' 

.c.o:
	$(CC) $(CFLAGS) -c $<

OBJS=	ael_gm.o    cross_gm.o  elds_lm.o   input_lm.o  main_lm.o	\
	ael_lm.o    dots_lm.o   eqs_lm.o    solve.o  	\
	avltree.o   dump_gm.o   lmeter.o          	\
	edget_gm.o  inpfiles.o  main_gm.o   rcnf_lm.o   show_ps.o

all: lmeter  

lmeter: $(OBJS) csspdms/csspdms.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o lmeter $(OBJS) csspdms/csspdms.a $(MALLOCDEBUG) $(THREADLIB) -lm -lc

csspdms/csspdms.a: csspdms/*.c csspdms/*.h
	cd csspdms; $(MAKE) $(MAKEDEFS) csspdms.a;

clean:
	-rm -f core *.s *.o *.a *~ *% lmeter
	cd csspdms; $(MAKE) $(MAKEDEFS) clean

install: lmeter
	cp -f lmeter ../../bin/

depend:
	$(CPP) $(CFLAGS) *.c > .depend
	cd csspdms; $(MAKE) $(MAKEDEFS) depend

test: lmeter
	(cd test; ../lmeter input.lm < test90.dxc)

#
# include a dependency file if one exists
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif
