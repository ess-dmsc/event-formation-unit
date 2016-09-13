
INCLUDE  = -I. -I../libs/include
CXXFLAGS = -O3 -std=c++11 -pthread $(INCLUDE)
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

bulkdatagen: bulkdatagen.cpp $(library) $(objects) Args.h Socket.h Timer.h
	$(CXX) $(CXXFLAGS) $< $(library) $(objects) -o $@

pipeline: pipeline.cpp $(library) Socket.h Timer.h Thread.h
	$(CXX) $(CXXFLAGS) $< $(library) -o $@

clean:
	@rm -f $(objects) $(PROGS) *~
