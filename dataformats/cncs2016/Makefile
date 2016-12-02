# Copyright (C) 2016 European Spallation Source ERIC

CFLAGS= -O3 -std=c++11 -Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs -I . -I ../../prototype2

prog = cncsread batchreader

all: $(prog)
	@echo done.

cncsread: cncsread.cpp
	g++ $(CFLAGS) cncsread.cpp -o $@


Analyse.o: Analyse.cpp Analyse.h
	g++ -c $(CFLAGS) Analyse.cpp

Args.o: Args.cpp Args.h
	g++ -c $(CFLAGS) Args.cpp

MapFile.o: MapFile.cpp MapFile.h
	g++ -c $(CFLAGS) MapFile.cpp

batchreader: batchreader.cpp Analyse.o Args.o MapFile.o
		g++ $(CFLAGS) batchreader.cpp Analyse.o Args.o MapFile.o \
      ../../prototype2/cspec/CSPECData.cpp \
		  ../../prototype2/cspec/CSPECChanConv.cpp -o $@

test: all
	@echo "Run on a single file"
	./$(prog) 2016_07_13_beamOn_4p96A_135.bin
	@echo "Run on multiple files"
	find . -name "*.bin" | xargs -n 1 ./$(prog) | grep "Double events"

clean:
	rm -f $(prog) *.o
