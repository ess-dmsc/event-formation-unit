# Copyright (C) 2016 European Spallation Source ERIC
#

.SUFFIXES:

ifndef _ARCH
  _ARCH := $(shell uname -s)
endif

VPATH = .. ../libs/include test

progs = efu2 gencspec gencspecfile dgmgjson cspec.so nmx.so udp.so
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

cspec.so.src =  cspec/cspec.cpp cspec/CSPECData.cpp cspec/CSPECChanConv.cpp
cspec.so.src += common/Producer.cpp
cspec.so.inc =  cspec/CSPECData.h cspec/CSPECChanConv.h common/Detector.h common/MultiGridGeometry.h
cspec.so.inc += gccintel.h common/Producer.h common/Stats.h common/Trace.h
cspec.so.inc +=

# For each standalone program add .src and .inc and .obj below
efu.src = common/EFUArgs.cpp efu/Loader.cpp efu/Launcher.cpp efu/pipeline.cpp
efu.src += efu/Server.cpp efu/Parser.cpp cspec/CalibrationFile.cpp
efu.inc = common/EFUArgs.h efu/Launcher.h efu/Loader.h common/Stats.h
efu.inc += efu/Server.h efu/Parser.h common/Trace.h cspec/CalibrationFile.h
efu.obj = $(efu.src:%.cpp=%.o)

gencspec.src = cspecgen/cspecgen.cpp cspecgen/CspecArgs.cpp cspec/CSPECData.cpp
gencspec.inc = cspecgen/CspecArgs.h cspec/CSPECData.h
gencspec.obj = $(gencspec.src:%.cpp=%.o)

gencspecfile.src = cspecgen/cspecfile.cpp cspecgen/CspecArgs.cpp
gencspecfile.inc = cspecgen/CspecArgs.h
gencspecfile.obj = $(gencspecfile.src:%.cpp=%.o)

dgmgjson.src = mgjson/dgmgjson.cpp mgjson/MGJsonArgs.cpp ../dataformats/cncs2016/src/RunSpecParse.cpp\
       ../dataformats/cncs2016/src/jsoncpp.cpp
dgmgjson.inc = mgjson/MGJsonArgs.h
dgmgjson.obj = $(dgmgjson.src:%.cpp=%.o)


.DEFAULT_GOAL = all

include makefiles/Makefile.config
include makefiles/Makefile.test

doxygen:
	mkdir -p doxygen
	doxygen doxyfile

all: $(progs)
	@echo done
	$(Q)chmod 440 *.so

$(efu.obj): $(efu.inc)
$(gencspec.obj): $(gencspec.inc)
$(gencspecfile.obj): $(gencspecfile.inc)

cspec.so: $(cspec.so.src) $(cspec.so.inc)
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
	$(Q)$(cxx) $(cxxflags) -I../dataformats/cncs2016/inc $< -c -o $@

$(library): Socket.h TSCTimer.h Timer.h SPSCFifo.h
	make -C ../libs

efu2: $(library) $(efu.obj)
	@echo building $@
	$(Q)$(cxx) $(cxxflags) $(efu.obj) $(library) $(ldflags) -o $@

gencspec: $(library) $(gencspec.obj) $(gencspec.inc)
	@echo building $@
	$(Q)$(cxx) $(cxxflags) $(gencspec.obj) $(library) $(ldflags) -o $@

gencspecfile: $(library) $(gencspecfile.obj) $(gencspecfile.inc)
	@echo building $@
	$(Q)$(cxx) $(gencspecfile.obj) $(library) $(ldflags) -o $@

dgmgjson: $(library) $(dgmgjson.obj) $(dgmgjson.inc)
	@echo building $@
	$(Q)$(cxx) $(dgmgjson.obj) $(library) $(ldflags) -o $@

realclean: clean
	make -C ../libs clean

doxygen-clean:
	@rm -fr doxygen doxygen.warnings

prog-clean:
	@rm -f $(efu.obj) $(gencspec.obj) $(dgmgjson.obj) $(gencspecfile.obj) $(progs) $(testprogs) *.o

test-clean:
	@rm -f *.log gmon.out *.xml *.valgrind
	@rm -fr html
	@find . -name "*.gcda" | xargs rm -f
	@find . -name "*.gcno" | xargs rm -f

clean: prog-clean test-clean doxygen-clean
	@rm -f  *~  gylle lcov.out CalibrationFileTest.*cal

release: realclean all clean test runtest
	../jenkins/style.bash
	@echo Ready for checkin
