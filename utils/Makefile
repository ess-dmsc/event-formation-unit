

progs=lehexdump32 memspdtst

all: $(progs)
	echo done

lehexdump32: lehxdump.cpp
	g++ --std=c++11 $< -o $@ 

memspdtst: memspdtst.c
	gcc $< -o $@ 
