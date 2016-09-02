
INCLUDE  = -I. -I../libs/include
CPPFLAGS = -std=c++11 $(INCLUDE)
CFLAGS   = $(INCLUDE)

VPATH = ../libs/include:../libs/Timer 

all: udpserver Server

Server: Socket.o Timer.o Server.cpp
	g++ $(CPPFLAGS) Server.cpp Socket.o Timer.o -o $@

udpserver: udpserver.c
	gcc $(CFLAGS) $< -o $@ 

Socket.o: Socket.cpp Socket.h
	g++ $(CPPFLAGS) -c  $<

Timer.o: Timer.cpp Timer.h
	g++ $(CPPFLAGS) -c  $< 

clean:
	rm -f udpserver *.o Server *~
