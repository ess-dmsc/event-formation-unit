

progs=lehexdump32 gencalibfile 

all: $(progs)
	echo done

lehexdump32: lehxdump.cpp
	g++ --std=c++11 $< -o $@ 

gencalibfile: gencalibfile.c
	g++ --std=c++11 $< -o $@ 
