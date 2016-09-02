


all: libs main.cpp
	g++ main.cpp Pipeline.cpp -I. -ggdb -ldl -o main

libs: liba.cpp liba.h libb.cpp libb.h
	g++ -I. -shared -ggdb liba.cpp -o nmx.so
	g++ -I. -shared -ggdb libb.cpp -o beer.so 

clean:
	rm -f main *.so
