
all:
	make -C src/lmeter
	make -C src/lm2sch
	make -C src/cif2dxc

clean:
	make -C src/lmeter clean
	make -C src/lm2sch clean
	make -C src/cif2dxc clean
	-rm -f src/scripts/*~ 

install:
	make -C src/lmeter install
	make -C src/lm2sch install
	make -C src/cif2dxc install
	cp -f src/scripts/* bin

distclean: clean
	-rm -f bin/*
	-rm -f `find . -name ".depend" -o -name "core" -o -name "a.out" -o -name "gmon.out" -o -name "*~"`
