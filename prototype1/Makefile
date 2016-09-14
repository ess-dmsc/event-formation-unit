
INCLUDE  = -I. -I../libs/include
WARNINGS = -Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs
OPTFLAGS = -O3
CXXFLAGS = $(OPTFLAGS) -std=c++11 -pthread $(INCLUDE) $(WARNINGS)
CFLAGS   = $(INCLUDE)

VPATH = ../libs/include

PROGS = bulkdatagen pipeline
library = ../libs/eventlib.a
source = Args.cpp
objects =$(source:%.cpp=%.o)

CXX=g++

all: $(PROGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -c

$(library):
	make -C ../libs

bulkdatagen: bulkdatagen.cpp $(library) $(objects) Args.h Socket.h Timer.h
	$(CXX) $(CXXFLAGS) $< $(library) $(objects) -o $@

pipeline: pipeline.cpp $(library) Socket.h Timer.h Thread.h
	$(CXX) $(CXXFLAGS) $< $(library) -o $@

clean:
	make -C ../libs clean
	@rm -f $(objects) $(PROGS) *~
