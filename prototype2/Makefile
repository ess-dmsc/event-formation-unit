# Copyright (C) 2016 European Spallation Source ERIC
#

.SUFFIXES:

ifndef _ARCH
  _ARCH := $(shell uname -s)
endif

VPATH = .. ../libs/include test

progs = efu2  cspec.so nmx.so udp.so cspec2.so nmxvmm2srs.so gencspec gencspecfile dgmgjson
# gennmxpcap removed as libpcap is not available on build node

library = ../libs/eventlib.a

# For each loadable detector add its .so.src and .so.inc dependencies
common.src = common/NewStats.cpp common/Producer.cpp common/FBSerializer.cpp ../dataformats/multigrid/src/DataSave.cpp
common.inc = common/Detector.h common/NewStats.h common/Producer.h common/FBSerializer.h
common.inc += common/Trace.h libs/include/gccintel.h common/EFUArgs.h
common.obj = $(common.src:%.cpp=%.o)

udp.so.src = udp/udp.cpp

nmx.so.src = NMX/nmx.cpp NMX/Clusterer.cpp NMX/EventNMX.cpp NMX/Eventlet.cpp nmxgen/EventletBuilderH5.cpp
nmx.so.inc = NMX/Clusterer.h NMX/EventNMX.h NMX/Eventlet.h nmxgen/EventletBuilderH5.h

nmxvmm2srs.so.src = nmxvmm2srs/nmxvmm2srs.cpp nmxvmm2srs/NMXVMM2SRSData.cpp NMX/Clusterer.cpp NMX/Geometry.cpp
nmxvmm2srs.so.src += NMX/Time.cpp NMX/EventNMX.cpp NMX/Eventlet.cpp nmxvmm2srs/EventletBuilder.cpp
nmxvmm2srs.so.src += nmxvmm2srs/HistSerializer.cpp
nmxvmm2srs.so.inc = NMX/Clusterer.h NMX/Geometry.h NMX/Time.h NMX/EventNMX.h NMX/Eventlet.h nmxvmm2srs/EventletBuilder.h
nmxvmm2srs.so.inc += nmxvmm2srs/NMXVMM2SRSData.h nmxvmm2srs/HistSerializer.h

cspec.so.src =  cspec/cspec.cpp cspec/CSPECData.cpp cspec/CSPECChanConv.cpp
cspec.so.inc =  cspec/CSPECData.h cspec/CSPECChanConv.h common/MultiGridGeometry.h

cspec2.so.src =  cspec2/cspec2.cpp cspec/CSPECData.cpp cspec/CSPECChanConv.cpp
cspec2.so.inc =  cspec/CSPECData.h cspec/CSPECChanConv.h common/MultiGridGeometry.h


# For each standalone program add .src and .inc and .obj below
efu.src = common/EFUArgs.cpp efu/Loader.cpp efu/Launcher.cpp efu/pipeline.cpp
efu.src += common/StatPublisher.cpp efu/ExitHandler.cpp
efu.src += efu/Server.cpp efu/Parser.cpp cspec/CalibrationFile.cpp
efu.inc = common/EFUArgs.h efu/Launcher.h efu/Loader.h
efu.inc += efu/Server.h efu/Parser.h common/Trace.h cspec/CalibrationFile.h
efu.inc += common/StatPublisher.h common/NewStats.h efu/ExitHandler.h

gencspec.src = cspecgen/cspecgen.cpp cspecgen/CspecArgs.cpp cspec/CSPECData.cpp
gencspec.inc = cspecgen/CspecArgs.h cspec/CSPECData.h

gencspecfile.src = cspecgen/cspecfile.cpp cspecgen/CspecArgs.cpp
gencspecfile.inc = cspecgen/CspecArgs.h

dgmgjson.src = mgjson/dgmgjson.cpp mgjson/MGJsonArgs.cpp ../dataformats/multigrid/src/RunSpecParse.cpp\
       ../dataformats/multigrid/src/jsoncpp.cpp
dgmgjson.inc = mgjson/MGJsonArgs.h


.DEFAULT_GOAL = all

include makefiles/Makefile.config
include makefiles/Makefile.test
include makefiles/Makefile.hdf5
include makefiles/Makefile.help

doxygen:
	mkdir -p doxygen
	doxygen doxyfile

all: printinfo $(progs)
	@echo done
	$(Q)chmod 664 *.so


cspec.so: $(cspec.so.src) $(common.obj) $(cspec.so.inc)
	@echo building specific $@
	$(Q)$(cxx) $(cxxflags) $(cxxflags.so) $($(@).src) $(common.obj) $(ldflags.so) -o $@

cspec2.so: $(cspec2.so.src) $(cspec2.so.inc) $(common.obj)
	@echo building specific $@
	$(Q)$(cxx) $(cxxflags) $(cxxflags.so) $($(@).src) $(common.obj) $(ldflags.so) -o $@

nmx.so: $(nmx.so.src) $(nmx.so.inc) $(common.obj)
	@echo building specific $@
	$(Q)$(cxx) $(cxxflags) $(cxxflags.so) $($(@).src) $(common.obj) $(ldflags.so) -o $@

nmxvmm2srs.so: $(nmxvmm2srs.so.src) $(nmxvmm2srs.so.inc) $(common.obj)
	@echo building specific $@
	$(Q)$(cxx) $(cxxflags) $(cxxflags.so) $($(@).src) $(common.obj) $(ldflags.so) -o $@

udp.so: $(udp.so.src) $(udp.so.inc) $(common.obj)
	@echo building specific $@
	$(Q)$(cxx) $(cxxflags) $(cxxflags.so) $($(@).src) $(common.obj) $(ldflags.so) -o $@


%.o: %.cpp
	@echo building $@
	$(Q)$(cxx) $(cxxflags) $(cxxflags.so) $< -c -o $@

$(library): Socket.h TSCTimer.h Timer.h SPSCFifo.h
	make -C ../libs

efu2: $(library) $(efu.src) $(efu.inc)
	@echo building $@
	$(Q)$(cxx) $(cxxflags) $(efu.src) $(library) $(ldflags) -o $@

gencspec: $(library) $(gencspec.src) $(gencspec.inc)
	@echo building $@
	$(Q)$(cxx) $(cxxflags) $(gencspec.src) $(library) $(ldflags) -o $@

gencspecfile: $(library) $(gencspecfile.src) $(gencspecfile.inc)
	@echo building $@
	$(Q)$(cxx) $(cxxflags) $(gencspecfile.src) $(library) $(ldflags) -o $@

gennmxpcap: $(library) nmxgen/wireshark.cpp nmxgen/NMXArgs.cpp nmxgen/ReaderPcap.cpp \
	      nmxgen/ReaderPcap.h nmxgen/NMXArgs.h
	$(Q)$(cxx) -std=c++11 -I. -I.. nmxgen/wireshark.cpp nmxgen/NMXArgs.cpp nmxgen/ReaderPcap.cpp $(library) -lpcap -o $@

dgmgjson: $(library) $(dgmgjson.src) $(dgmgjson.inc)
	@echo building $@
	$(Q)$(cxx) $(cxxflags) $(dgmgjson.src) $(library) $(ldflags) -o $@

realclean: clean
	make -C ../libs clean

doxygen-clean:
	@rm -fr doxygen doxygen.warnings

prog-clean:
	@rm -f $(efu.obj) $(gencspec.obj) $(dgmgjson.obj) $(gencspecfile.obj) \
	       $(gennmxfile.obj) $(progs) $(testprogs) *.o

test-clean:
	@rm -f *.log gmon.out *.xml *.valgrind
	@rm -fr html
	@find . -name "*.gcda" | xargs rm -f
	@find . -name "*.gcno" | xargs rm -f

clean: prog-clean test-clean doxygen-clean
	@rm -f  *~  gylle lcov.out CalibrationFileTest.*cal */*.o

release: realclean all clean test runtest
	../jenkins/style.bash
	@echo Ready for checkin
