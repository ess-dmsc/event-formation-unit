
INCLUDE  = -I. -I../libs/include
CPPFLAGS = -O3 -std=c++11 -pthread $(INCLUDE)
CFLAGS   = $(INCLUDE)

VPATH = ../libs/include:../libs/Timer:../libs/Socket:

PROGS = bulkdatagen pipeline
OBJECTS = Socket.o Timer.o

all: $(PROGS)

bulkdatagen: bulkdatagen.cpp $(OBJECTS) Socket.h Timer.h
	g++ $(CPPFLAGS) $< $(OBJECTS) -o $@

pipeline: pipeline.cpp $(OBJECTS) Socket.h Timer.h Thread.h
	g++ $(CPPFLAGS) $< $(OBJECTS) -o $@

%.o:%.cpp
	g++ $(CPPFLAGS) -c  $<

clean:
	@rm -f $(OBJECTS) $(PROGS) *~
