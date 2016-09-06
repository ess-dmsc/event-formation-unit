

Q=@
ifdef V
Q=
endif

CXXFLAGS = -O3 -std=c++11 -I../include -I/usr/include

all: testmain
	@echo done

testmain: testmain.cpp testEventBasic.o ../Timer/Timer.cpp
	g++ $(CXXFLAGS) $< testEventBasic.o ../Timer/Timer.cpp -o $@ -lgtest -pthread

testEventBasic.o: testEventBasic.cpp
	$(Q)g++  $(CXXFLAGS) -c $<

clean:
	rm -f testmain *.o 
