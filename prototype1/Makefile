
INCLUDE  = -I. -I../libs/include
CPPFLAGS = -O3 -std=c++11 -pthread $(INCLUDE)
CFLAGS   = $(INCLUDE)

VPATH = ../libs/include

PROGS = bulkdatagen pipeline
library = ../libs/eventlib.a

all: $(PROGS)

bulkdatagen: bulkdatagen.cpp $(library) Socket.h Timer.h
	g++ $(CPPFLAGS) $< $(library) -o $@

pipeline: pipeline.cpp $(library) Socket.h Timer.h Thread.h
	g++ $(CPPFLAGS) $< $(library) -o $@

clean:
	@rm -f $(OBJECTS) $(PROGS) *~
