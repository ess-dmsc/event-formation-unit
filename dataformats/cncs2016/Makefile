# Copyright (C) 2016 European Spallation Source ERIC

CFLAGS= -O3 -std=c++11 -Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs -I src -I ../../prototype2

prog = cncsread batchreader

all: $(prog)
	@echo done.

cncsread: src/cncsread.cpp
	g++ $(CFLAGS) src/cncsread.cpp -o $@


Analyse.o: src/Analyse.cpp src/Analyse.h
	g++ -c $(CFLAGS) src/Analyse.cpp

Args.o: src/Args.cpp src/Args.h
	g++ -c $(CFLAGS) src/Args.cpp

MapFile.o: src/MapFile.cpp src/MapFile.h
	g++ -c $(CFLAGS) src/MapFile.cpp

batchreader: src/batchreader.cpp Analyse.o Args.o MapFile.o
		g++ $(CFLAGS) src/batchreader.cpp Analyse.o Args.o MapFile.o \
      ../../prototype2/cspec/CSPECData.cpp \
		  ../../prototype2/cspec/CSPECChanConv.cpp -o $@

test: all
	@echo "Run on a single file"
	./$(prog) 2016_07_13_beamOn_4p96A_135.bin
	@echo "Run on multiple files"
	find . -name "*.bin" | xargs -n 1 ./$(prog) | grep "Double events"

clean:
	rm -f $(prog) *.o
