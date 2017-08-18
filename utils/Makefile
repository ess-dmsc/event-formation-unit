

progs=lehexdump32 gencalibfile tsctest

all: $(progs)
	@echo done

lehexdump32: lehxdump.cpp
	g++ --std=c++11 $< -o $@

gencalibfile: gencalibfile.c
	g++ --std=c++11 $< -o $@

tsctest: testtsc.cpp
	g++ --std=c++11 -O3 -march=native $< -o $@

clean:
	rm -f $(progs)
