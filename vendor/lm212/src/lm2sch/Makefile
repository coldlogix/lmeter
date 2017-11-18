CC=gcc
CFLAGS = -O6 -g 
LDFLAGS = 

lm2sch: lm2sch.o linalg.o
	$(CC) $(CFLAGS) $(LDFLAGS)  -o lm2sch lm2sch.o linalg.o -lm 

install: lm2sch
	cp -f lm2sch ../../bin

clean: 
	-rm -f  core *.s *.o *.a *~ lm2sch
