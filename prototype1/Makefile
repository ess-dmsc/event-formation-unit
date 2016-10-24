# Copyright (C) 2016 European Spallation Source
#

include Makefile.config

VPATH = ../libs/include

PROGS = bulkdatagen efu
library = ../libs/eventlib.a

src.datagen = DGArgs.cpp ../prototype2/cspec/CSPECData.cpp
obj.datagen =$(src.datagen:%.cpp=%.o)

src.efu = EFUArgs.cpp Producer.cpp
obj.efu = $(src.efu:%.cpp=%.o)

all: $(PROGS)

%.o: %.cpp
	echo compiling
	$(cxx) $(cxxflags) $< -c -o $@ $(prof)

$(library):
	make -C ../libs

bulkdatagen: bulkdatagen.cpp $(library) $(obj.datagen) DGArgs.h Socket.h Timer.h
	$(cxx) $(cxxflags) $< $(library) $(obj.datagen) -o $@

efu: pipeline.cpp $(library) $(obj.efu) EFUArgs.h Socket.h Timer.h Thread.h Producer.h
	$(cxx) $(cxxflags) $(prof) $< $(library) $(obj.efu) $(ldflags) -o $@

realclean: clean
	make -C ../libs clean

clean:
	@rm -f $(obj.efu) $(obj.datagen) $(PROGS) *.log *~ *.gc* gmon.out
