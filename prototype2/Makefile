# Copyright (C) 2016 European Spallation Source
#

include Makefile.config

VPATH = ../libs/include

PROGS = efu2 nmx.so cspec.so
library = ../libs/eventlib.a

src.efu = EFUArgs.cpp Loader.cpp
obj.efu = $(src.efu:%.cpp=%.o)

all: $(PROGS)

nmx.so: nmx.cpp Detector.h
	$(cxx) -fPIC -shared $< -o $@
	chmod 440 $@

cspec.so: cspec.cpp Detector.h
	$(cxx) -fPIC -shared $< -o $@
	chmod 440 $@

%.o: %.cpp
	$(cxx) $(cxxflags) $< -c -o $@ $(prof)

$(library):
	make -C ../libs

efu2: pipeline.cpp $(library) $(obj.efu) EFUArgs.h Socket.h Timer.h Thread.h
	$(cxx) $(cxxflags) $(prof) $< $(library) $(obj.efu) $(ldflags) -o $@

realclean: clean
	make -C ../libs clean

clean:
	@rm -f $(obj) $(PROGS) *.log *~ *.gc* gmon.out *.o
