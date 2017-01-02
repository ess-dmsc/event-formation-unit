# Copyright (C) 2016 European Spallation Source ERIC

ldflags_test  = $(ldflags) -lgtest -pthread
cxxflags_test = $(cxxflags) -I inc -I/usr/include

Q=@
ifdef V
Q=
endif

CFLAGS= -O3 -std=c++11 -Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs -I inc -I ../../prototype2

prog = cncsread batchreader genpixids
testprogs = PeakFinderTest HistogramTest

all: $(prog)
	@echo done.

cncsread: src/cncsread.cpp
	@echo building $@
	$(Q)g++ $(CFLAGS) $< -o $@

genpixids: src/genpixids.cpp ../../prototype2/cspec/CalibrationFile.cpp \
						../../prototype2/cspec/CSPECChanConv.cpp MapFile.o DataSave.o
	@echo building $@
	$(Q)g++ $(CFLAGS) ../../prototype2/cspec/CalibrationFile.cpp \
						../../prototype2/cspec/CSPECChanConv.cpp MapFile.o DataSave.o $< -o $@

jsoncpp.o: src/jsoncpp.cpp inc/json.h
	@echo building $@
	$(Q)g++ -c $(CFLAGS) $<

Analyse.o: src/Analyse.cpp inc/Analyse.h
	@echo building $@
	$(Q)g++ -c $(CFLAGS) $<

Args.o: src/Args.cpp inc/Args.h
	@echo building $@
	$(Q)g++ -c $(CFLAGS) $<

MapFile.o: src/MapFile.cpp inc/MapFile.h
	@echo building $@
	$(Q)g++ -c $(CFLAGS) $<

PeakFinder.o: src/PeakFinder.cpp inc/PeakFinder.h
	@echo building $@
	$(Q)g++ -c $(CFLAGS) -I../.. $<

RunSpecParse.o: src/RunSpecParse.cpp inc/RunSpecParse.h
		@echo building $@
		$(Q)g++ -c $(CFLAGS) -I../.. $<

DataSave.o: src/DataSave.cpp inc/DataSave.h
		@echo building $@
		$(Q)g++ -c $(CFLAGS) -I../.. $<

batchreader: src/batchreader.cpp inc/RawDataFiles.h Analyse.o Args.o MapFile.o \
	           PeakFinder.o jsoncpp.o RunSpecParse.o
	@echo building $@
	$(Q)g++ $(CFLAGS) src/batchreader.cpp \
	    Analyse.o Args.o MapFile.o PeakFinder.o RunSpecParse.o jsoncpp.o  \
      ../../prototype2/cspec/CSPECData.cpp ../../prototype2/cspec/CalibrationFile.cpp \
		  ../../prototype2/cspec/CSPECChanConv.cpp -o $@

PeakFinderTest: src/test/PeakFinderTest.cpp src/PeakFinder.cpp inc/PeakFinder.h
	@echo building $@
	$(Q)g++ $(CFLAGS) $(cxxflags_test) -o $@ -I../.. -I../../prototype2/test src/test/PeakFinderTest.cpp src/PeakFinder.cpp $(ldflags_test)

HistogramTest: src/test/HistogramTest.cpp inc/Histogram.h
	@echo building $@
	$(Q)g++ $(CFLAGS) $(cxxflags_test) -o $@ -I../../prototype2/test src/test/HistogramTest.cpp $(ldflags_test)

test: $(testprogs)
	echo done

runtest: test
	for test in $(testprogs) ; do ./$$test --gtest_output=xml:$$test.xml ; done

clean:
	rm -f $(prog) *.o $(testprogs) *.xml
