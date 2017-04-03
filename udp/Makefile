
INCLUDE  = -I. -I..
CXXFLAGS = -O3 -std=c++11 $(INCLUDE)
CFLAGS   = $(INCLUDE)

VPATH = ../libs/include

PROGS = udprx udptx
library=../libs/eventlib.a
sources = Args.cpp
objects = $(sources:%.cpp=%.o)

all: $(PROGS)

udprx: udprx.cpp $(library) $(objects) Socket.h Timer.h
	g++ $(CXXFLAGS) $< $(objects) $(library) -o $@

udptx: udptx.cpp $(library) $(objects) Args.h Socket.h Timer.h
	g++ $(CXXFLAGS) $< $(objects) $(library) -o $@

%.o: %.cpp
	g++ $(CXXFLAGS) -c $<


clean:
	@rm -f $(objects) $(PROGS) *~
