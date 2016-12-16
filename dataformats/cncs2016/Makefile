# Copyright (C) 2016 European Spallation Source ERIC

# Specify alternative googletest library (suppress warnings on Jenkins)
ifndef GTEST
ldflags_test  = $(ldflags) -lgtest -pthread
cxxflags_test = $(cxxflags) -I/usr/include
else
ldflags_test  = $(ldflags) $(GTEST)/lib/libgtest.a -pthread
cxxflags_test = $(cxxflags) -I$(GTEST)/include -Wno-error=sign-compare
endif

Q=@
ifdef V
Q=
endif

CFLAGS= -O3 -std=c++11 -Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs -I src -I ../../prototype2

prog = cncsread batchreader genpixids
testprogs = PeakFinderTest HistogramTest

all: $(prog)
	@echo done.

cncsread: src/cncsread.cpp
	@echo building $@
	$(Q)g++ $(CFLAGS) $< -o $@

genpixids: src/genpixids.cpp ../../prototype2/cspec/CalibrationFile.cpp \
						../../prototype2/cspec/CSPECChanConv.cpp MapFile.o
	@echo building $@
	$(Q)g++ $(CFLAGS) ../../prototype2/cspec/CalibrationFile.cpp \
						../../prototype2/cspec/CSPECChanConv.cpp MapFile.o $< -o $@


Analyse.o: src/Analyse.cpp src/Analyse.h
	@echo building $@
	$(Q)g++ -c $(CFLAGS) $<

Args.o: src/Args.cpp src/Args.h
	@echo building $@
	$(Q)g++ -c $(CFLAGS) $<

MapFile.o: src/MapFile.cpp src/MapFile.h
	@echo building $@
	$(Q)g++ -c $(CFLAGS) $<

PeakFinder.o: src/PeakFinder.cpp src/PeakFinder.h
	@echo building $@
	$(Q)g++ -c $(CFLAGS) -I../.. $<

batchreader: src/batchreader.cpp src/RawDataFiles.h Analyse.o Args.o MapFile.o PeakFinder.o
	@echo building $@
	$(Q)g++ $(CFLAGS) src/batchreader.cpp Analyse.o Args.o MapFile.o PeakFinder.o \
      ../../prototype2/cspec/CSPECData.cpp ../../prototype2/cspec/CalibrationFile.cpp \
		  ../../prototype2/cspec/CSPECChanConv.cpp -o $@

PeakFinderTest: src/test/PeakFinderTest.cpp src/PeakFinder.cpp src/PeakFinder.h
	@echo building $@
	$(Q)g++ $(CFLAGS) $(cxxflags_test) -o $@ -I../.. -I../../prototype2/test src/test/PeakFinderTest.cpp src/PeakFinder.cpp $(ldflags_test)

HistogramTest: src/test/HistogramTest.cpp src/Histogram.h
	@echo building $@
	$(Q)g++ $(CFLAGS) $(cxxflags_test) -o $@ -I../../prototype2/test src/test/HistogramTest.cpp $(ldflags_test)

test: $(testprogs)
	echo done

runtest: test
	for test in $(testprogs) ; do ./$$test --gtest_output=xml:$$test.xml ; done

clean:
	rm -f $(prog) *.o $(testprogs) *.xml
