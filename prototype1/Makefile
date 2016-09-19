#
#

cxx=g++

include  = -I. -I../libs/include
warnings = -Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs
optflags = -O3
ldflags = -L/usr/local/lib -lrdkafka++ -lrdkafka -lpthread -lrt

cxxflags = -std=c++11 $(optflags)  $(include) $(warnings)

VPATH = ../libs/include

PROGS = bulkdatagen efu
library = ../libs/eventlib.a

src.datagen = DGArgs.cpp
obj.datagen =$(src.datagen:%.cpp=%.o)

src.efu = EFUArgs.cpp Producer.cpp
obj.efu = $(src.efu:%.cpp=%.o)

all: $(PROGS)

%.o: %.cpp
	$(cxx) $(cxxflags) $< -c -o $@

$(library):
	make -C ../libs

bulkdatagen: bulkdatagen.cpp $(library) $(obj.datagen) DGArgs.h Socket.h Timer.h
	$(cxx) $(cxxflags) $< $(library) $(obj.datagen) -o $@

efu: pipeline.cpp $(library) $(obj.efu) EFUArgs.h Socket.h Timer.h Thread.h Producer.h
	$(cxx) $(cxxflags) $< $(library) $(obj.efu) $(ldflags) -o $@

realclean: clean
	make -C ../libs clean

clean:
	@rm -f $(obj.efu) $(obj.datagen) $(PROGS) *~
