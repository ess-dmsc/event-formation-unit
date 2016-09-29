

prog = cncsread

all: $(prog)
	gcc main.c -o $(prog)


test: all
	@echo "Run on a single file"
	./$(prog) 2016_07_13_beamOn_4p96A_135.bin
	@echo "Run on multiple files"
	find . -name "*.bin" | xargs -n 1 ./$(prog) | grep "Double events"
