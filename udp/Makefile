
INCLUDE  = -I. -I../libs/include
CPPFLAGS = -O3 -std=c++11 $(INCLUDE)
CFLAGS   = $(INCLUDE)

VPATH = ../libs/include:../libs/Timer 

PROGS = udprx udptx
OBJECTS = Socket.o Timer.o

all: $(PROGS)

udprx: udprx.cpp $(OBJECTS) Socket.h Timer.h 
	g++ $(CPPFLAGS) $< $(OBJECTS) -o $@

udptx: udptx.cpp $(OBJECTS) Socket.h Timer.h
	g++ $(CPPFLAGS) $< $(OBJECTS) -o $@

%.o:%.cpp
	g++ $(CPPFLAGS) -c  $<

clean:
	@rm -f $(OBJECTS) $(PROGS) *~
