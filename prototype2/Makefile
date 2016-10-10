# Copyright (C) 2016 European Spallation Source
#

include Makefile.config

VPATH = ../libs/include

progs = efu2 nmx.so cspec.so udpraw.so
testprogs = testdetector
library = ../libs/eventlib.a

src.efu = EFUArgs.cpp Loader.cpp Launcher.cpp
obj.efu = $(src.efu:%.cpp=%.o)

all: $(progs)

test: $(testprogs)

testdetector: test/testDetector.cpp
	g++ $(cxxflags) $^ -o $@ $(ldflags) -lgtest -pthread

nmx.so: nmx.cpp Detector.h
	$(cxx) $(cxxflags) -fPIC -shared $< -o $@
	chmod 440 $@

udpraw.so: udpraw.cpp $(library) Detector.h Socket.h Timer.h
	$(cxx) $(cxxflags) -Wl,--whole-archive $(library) -Wl,--no-whole-archive -fPIC -shared $< -o $@
	chmod 440 $@

cspec.so: cspec.cpp Detector.h
	$(cxx) $(cxxflags) -fPIC -shared $< -o $@
	chmod 440 $@

%.o: %.cpp
	$(cxx) $(cxxflags) $< -c -o $@ $(prof)

$(library):
	make -C ../libs

efu2: pipeline.cpp $(library) $(obj.efu) EFUArgs.h Socket.h Timer.h Launcher.h Loader.h
	$(cxx) $(cxxflags) $(prof) $< $(library) $(obj.efu) $(ldflags) -o $@

realclean: clean
	make -C ../libs clean

clean:
	@rm -f $(obj) $(progs) $(testprogs) *.log *~ *.gc* gmon.out *.o
