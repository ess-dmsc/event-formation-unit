


CXXFLAGS  = -Iinclude -std=c++11 -O3 -fPIC
LDFLAGS   = -L. -lpthread
CXXFLAGS += -Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs -Wno-error=sign-compare -Wno-error=conversion-null

Q=@
ifdef V
Q=
endif

ifndef GTEST
LDFLAGS += -lgtest
CXXFLAGS+= -I/usr/include
else
LDFLAGS += $(GTEST)/lib/libgtest.a
CXXFLAGS+= -I$(GTEST)/include
endif

ifdef COV
CXXFLAGS += -coverage
endif

hdrs    = include/Socket.h include/Timer.h include/Thread.h include/Counter.h
sources = source/Socket.cpp source/Timer.cpp source/Thread.cpp
objects = $(sources:%.cpp=%.o)
library = eventlib.a

testprogs  = testevent testpqspeed testpqbasic testpqmin
testprogs += testcounter

all: $(library) $(hdrs)

$(library): $(objects)
	ar r $@ $^
	ranlib $@

test: $(testprogs)

testevent: test/NMXEvent/testmain.cpp test/NMXEvent/testEventBasic.cpp $(library)
	g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -pthread

testpqbasic: test/PQ/testmain.cpp test/PQ/testPQBasic.cpp $(library)
	g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS)  -pthread

testpqspeed: test/PQ/testmain.cpp test/PQ/testPQSpeed.cpp $(library)
	g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS)  -pthread

testpqmin: test/PQ/testmain.cpp test/PQ/testPQMin.cpp $(library)
		g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -pthread

testcounter: test/Counter/testmain.cpp test/Counter/testCounter.cpp $(library)
		g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -pthread

clean:
	rm -f $(objects) $(library) $(testprogs) *.gc*
