# Copyright (C) 2016 European Spallation Source
#

VPATH = ../libs/include test

progs = efu2 cspecgen2 cspec.so nmx.so udp.so
library = ../libs/eventlib.a

# For each loadable detector add its .so.src and .so.inc dependencies
udp.so.src = udp/udp.cpp
udp.so.inc = common/Detector.h

nmx.so.src = NMX/nmx.cpp
nmx.so.inc = common/Detector.h
ifndef NOKAFKA
nmx.so.src += common/Producer.cpp
nmx.so.inc += common/Producer.h
endif

cspec.so.src = cspec/cspec.cpp cspec/CSPECData.cpp cspec/CSPECChanConv.cpp  common/RingBuffer.cpp
cspec.so.inc = cspec/CSPECData.h cspec/CSPECChanConv.h common/Detector.h gccintel.h

# For each standalone program add .src and .inc and .obj below
efu.src = common/EFUArgs.cpp efu/Loader.cpp efu/Launcher.cpp efu/pipeline.cpp
efu.inc = common/EFUArgs.h efu/Launcher.h efu/Loader.h
efu.obj = $(efu.src:%.cpp=%.o)

cspecgen.src = cspecgen/cspecgen.cpp cspecgen/CspecArgs.cpp cspec/CSPECData.cpp
cspecgen.inc = cspecgen/CspecArgs.h cspec/CSPECData.h
cspecgen.obj = $(cspecgen.src:%.cpp=%.o)

.DEFAULT_GOAL = all

include makefiles/Makefile.config
include makefiles/Makefile.test

all: $(progs)
	@echo done
	$(Q)chmod 440 *.so

cspec.so: $(cspec.so.src) $(cspc.so.inc)
	@echo building specific $@
	$(Q)$(cxx) $(cxxflags) $($(@).src) $(ldflags.so) -o $@

nmx.so: $(nmx.so.src) $(nmx.so.inc)
	@echo building specific $@
	$(Q)$(cxx) $(cxxflags) $($(@).src) $(ldflags.so) -o $@

udp.so: $(udp.so.src) $(udp.so.inc)
	@echo building specific $@
	$(Q)$(cxx) $(cxxflags) $($(@).src) $(ldflags.so) -o $@

%.o: %.cpp
	@echo building $@
	$(Q)$(cxx) $(cxxflags) $< -c -o $@

$(library): Socket.h Timer.h Counter.h
	make -C ../libs

efu2: $(library) $(efu.obj) $(efu.inc)
	@echo building $@
	$(Q)$(cxx) $(cxxflags) $(efu.obj) $(library) $(ldflags) -o $@

cspecgen2: $(library) $(cspecgen.obj) $(cspecgen.inc)
		@echo building $@
		$(Q)$(cxx) $(cxxflags) $(cspecgen.obj) $(library) $(ldflags) -o $@

realclean: clean
	make -C ../libs clean

clean:
	@rm -f $(efu.obj) $(progs) $(testprogs) *.log *~ gmon.out *.o gylle *.xml
	@rm -fr html lcov.out *.valgrind
	@find . -name "*.gc*" | xargs rm -f
