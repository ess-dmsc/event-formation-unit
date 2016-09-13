
Q=@
ifdef V
Q=
endif

CXXFLAGS = -Iinclude -std=c++11 -O3
LDFLAGS = -L.

ifdef COV
CXXFLAGS += -coverage
endif

hdrs    =
sources = source/Socket.cpp source/Timer.cpp source/Thread.cpp
objects = $(sources:%.cpp=%.o)
library = eventlib.a

testprogs = testevent testpqspeed testpqbasic testpqmin

all: $(library)

$(library): $(objects)
	ar r $@ $^
	ranlib $@

test: $(testprogs)

testevent: NMXEvent/testmain.cpp NMXEvent/testEventBasic.cpp $(library)
	g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lgtest -pthread

testpqbasic: PQ/testmain.cpp PQ/testPQBasic.cpp $(library)
	g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lgtest -pthread

testpqspeed: PQ/testmain.cpp PQ/testPQSpeed.cpp $(library)
	g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lgtest -pthread

testpqmin: PQ/testmain.cpp PQ/testPQMin.cpp $(library)
		g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lgtest -pthread

clean:
	rm -f $(objects) $(library) $(testprogs) *.gc*
