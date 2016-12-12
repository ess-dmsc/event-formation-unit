

CXXFLAGS  = -I.. -std=c++11 -O3 -fPIC
LDFLAGS   = -L. -lpthread
CXXFLAGS += -Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs -Wno-error=sign-compare -Wno-error=conversion-null

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
hdrs+= include/TSCTimer.h include/Counter.h include/StatCounter.h
sources = source/Socket.cpp source/Timer.cpp source/Thread.cpp source/TSCTimer.cpp
objects = $(sources:%.cpp=%.o)
library = eventlib.a

testprogs  = NMXEventTest PQSpeedTest PQBasicTest PQMinTest CounterTest StatCounterTest
# StatCounterTest

all: $(library)

$(library): $(objects) $(hdrs)
	ar r $@ $^
	ranlib $@

test: $(testprogs)

runtest: test
	for test in $(testprogs) ; do ./$$test --gtest_output=xml:$$test.xml ; done

NMXEventTest: test/NMXEvent/testmain.cpp test/NMXEvent/NMXEventTest.cpp $(library)
	g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -pthread

PQBasicTest: test/PQ/testmain.cpp test/PQ/PQBasicTest.cpp $(library)
	g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS)  -pthread

PQSpeedTest: test/PQ/testmain.cpp test/PQ/PQSpeedTest.cpp $(library)
	g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS)  -pthread

PQMinTest: test/PQ/testmain.cpp test/PQ/PQMinTest.cpp $(library)
	g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -pthread

CounterTest: test/Counter/testmain.cpp test/Counter/CounterTest.cpp $(library)
	g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -pthread

StatCounterTest: test/Counter/testmain.cpp test/Counter/StatCounterTest.cpp $(library)
	g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -pthread

clean:
	rm -f $(objects) $(library) $(testprogs) *.gc* *.valgrind *.xml
