#
#

CXX=g++

INCLUDE  = -I. -I../libs/include
WARNINGS = -Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs
OPTFLAGS = -O3
CXXFLAGS = $(OPTFLAGS) -std=c++11 -pthread $(INCLUDE) $(WARNINGS)
CFLAGS   = $(INCLUDE)

VPATH = ../libs/include

PROGS = bulkdatagen efu
library = ../libs/eventlib.a

src.datagen = DGArgs.cpp
obj.datagen =$(src.datagen:%.cpp=%.o)

src.efu = EFUArgs.cpp
obj.efu = $(src.efu:%.cpp=%.o)

all: $(PROGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@

$(library):
	make -C ../libs

bulkdatagen: bulkdatagen.cpp $(library) $(obj.datagen) DGArgs.h Socket.h Timer.h
	$(CXX) $(CXXFLAGS) $< $(library) $(obj.datagen) -o $@

efu: pipeline.cpp $(library) $(obj.efu) EFUArgs.h Socket.h Timer.h Thread.h
	$(CXX) $(CXXFLAGS) $< $(library) $(obj.efu) -o $@

realclean: clean
	make -C ../libs clean

clean:
	@rm -f $(obj.efu) $(obj.datagen) $(PROGS) *~
