# Copyright (C) 2016 European Spallation Source ERIC

# Specify alternative googletest library (suppress warnings on Jenkins)
ifndef GTEST
ldflags_test  = $(ldflags) -lgtest -pthread
cxxflags_test = $(cxxflags) -I/usr/include
else
ldflags_test  = $(ldflags) $(GTEST)/lib/libgtest.a -pthread
cxxflags_test = $(cxxflags) -I$(GTEST)/include -Wno-error=sign-compare
endif

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

PeakFinder.o: src/PeakFinder.cpp src/PeakFinder.h
	g++ -c $(CFLAGS) -I../.. src/PeakFinder.cpp

batchreader: src/batchreader.cpp Analyse.o Args.o MapFile.o PeakFinder.o
		g++ $(CFLAGS) src/batchreader.cpp Analyse.o Args.o MapFile.o PeakFinder.o \
      ../../prototype2/cspec/CSPECData.cpp \
		  ../../prototype2/cspec/CSPECChanConv.cpp -o $@

PeakFinderTest: src/test/PeakFinderTest.cpp src/PeakFinder.cpp src/PeakFinder.h
	g++ $(CFLAGS) $(cxxflags_test) -o $@ -I../.. -I../../prototype2/test src/test/PeakFinderTest.cpp src/PeakFinder.cpp $(ldflags_test)

HistogramTest: src/test/HistogramTest.cpp src/Histogram.h
	g++ $(CFLAGS) $(cxxflags_test) -o $@ -I../../prototype2/test src/test/HistogramTest.cpp $(ldflags_test)


test: PeakFinderTest HistogramTest
	echo done

runtest: all
	@echo "Run on a single file"
	./$(prog) 2016_07_13_beamOn_4p96A_135.bin
	@echo "Run on multiple files"
	find . -name "*.bin" | xargs -n 1 ./$(prog) | grep "Double events"

clean:
	rm -f $(prog) *.o PeakFinderTest HistogramTest
