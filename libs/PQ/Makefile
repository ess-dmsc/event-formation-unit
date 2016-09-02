

Q=@
ifdef V
Q=
endif

CXXFLAGS = -O3 -std=c++11 -I../include -I/usr/include

VPATH = ../Timer

all: testmain
	@echo done

testmain: testmain.cpp testPQSpeed.o testPQMin.o testPQBasic.o Timer.cpp
	g++ $(CXXFLAGS) $< testPQSpeed.o testPQMin.o testPQBasic.o ../Timer/Timer.cpp -o $@ -lgtest -pthread

testPQMin.o: testPQMin.cpp
	$(Q)g++  $(CXXFLAGS) -c $<

testPQBasic.o: testPQBasic.cpp
	$(Q)g++  $(CXXFLAGS) -c $<

testPQSpeed.o: testPQSpeed.cpp
	$(Q)g++  $(CXXFLAGS) -c $<  

clean:
	rm -f testmain *.o 
