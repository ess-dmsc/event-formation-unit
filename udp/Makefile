
INCLUDE  = -I. -I../libs/include
CPPFLAGS = -O3 -std=c++11 $(INCLUDE)
CFLAGS   = $(INCLUDE)

VPATH = ../libs/include

PROGS = udprx udptx
library=../libs/eventlib.a

all: $(PROGS)

udprx: udprx.cpp $(library) Socket.h Timer.h
	g++ $(CPPFLAGS) $< $(OBJECTS) $(library) -o $@

udptx: udptx.cpp $(library) Socket.h Timer.h
	g++ $(CPPFLAGS) $< $(OBJECTS) $(library) -o $@


clean:
	@rm -f $(OBJECTS) $(PROGS) *~
