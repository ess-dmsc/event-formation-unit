

CFLAGS=-Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs --std=c++11

prog = cncsread analyse

all: $(prog)
	@echo done.

cncsread: main.cpp
	gcc $(CFLAGS) main.cpp -o $@

analyse: analyse.cpp ../../prototype2/cspec/CSPECData.cpp ../../prototype2/cspec/CSPECChanConv.cpp 
		g++ $(CFLAGS) analyse.cpp -I ../../prototype2 ../../prototype2/cspec/CSPECData.cpp \
		../../prototype2/cspec/CSPECChanConv.cpp -o $@

test: all
	@echo "Run on a single file"
	./$(prog) 2016_07_13_beamOn_4p96A_135.bin
	@echo "Run on multiple files"
	find . -name "*.bin" | xargs -n 1 ./$(prog) | grep "Double events"

clean:
	rm -f $(prog)
