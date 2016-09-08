#
#

Q=@
ifdef V
Q=
endif

CXXFLAGS = -O3 -std=c++11 -I../include -I/usr/include 

ifdef COV
CXXFLAGS += -coverage
endif

VPATH = ../Timer

all: testmain
	@echo done

%.o: %.c
	$(Q)g++  $(CXXFLAGS) -c $<

testmain: testmain.cpp testPQSpeed.o testPQMin.o testPQBasic.o Timer.cpp
	g++ $(CXXFLAGS) $< testPQSpeed.o testPQMin.o testPQBasic.o ../Timer/Timer.cpp -o $@ -lgtest -pthread

testrun: testmain
	./testmain

build:
	mkdir build 

coverage: build
	lcov -c -b . -d . -o build/lcov.out
	genhtml -o build build/lcov.out

clean:
	rm -fr testmain *.o *.gc* build
