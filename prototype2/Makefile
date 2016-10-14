# Copyright (C) 2016 European Spallation Source
#

VPATH = ../libs/include test

progs = efu2 nmx.so cspec.so udpraw.so udprawtx.so
nmx.so.src = Producer.cpp
nmx.so.inc = Producer.h
cspec.so.src = CSPECData.cpp CSPECChanConv.cpp
cspec.so.inc = CSPECData.h CSPECChanConv.h

library = ../libs/eventlib.a

src.efu = EFUArgs.cpp Loader.cpp Launcher.cpp
obj.efu = $(src.efu:%.cpp=%.o)

.DEFAULT_GOAL = all

include Makefile.config
include Makefile.test

all: $(progs)
	@echo done
	$(Q)chmod 440 *.so

cspec.so: cspec.cpp Detector.h $(cspec.so.src) $(cspc.so.inc)
	@echo building specific $@
	$(Q)$(cxx) $(cxxflags) $($(@).src) $(ldflags.so) $< -o $@

%.so: %.cpp Detector.h
	@echo building $@
	$(Q)$(cxx) $(cxxflags) $($(@).src) $(ldflags.so) $< -o $@

%.o: %.cpp
	@echo building $@
	$(Q)$(cxx) $(cxxflags) $< -c -o $@ $(prof)

$(library): Socket.h Timer.h Counter.h
	make -C ../libs

efu2: pipeline.cpp $(library) $(obj.efu) EFUArgs.h Launcher.h Loader.h
	@echo building $@
	$(Q)$(cxx) $(cxxflags) $(prof) $< $(library) $(obj.efu) $(ldflags) -o $@

realclean: clean
	make -C ../libs clean

clean:
	@rm -f $(obj) $(progs) $(testprogs) *.log *~ *.gc* gmon.out *.o gylle *.xml
