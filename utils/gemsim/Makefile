

PROG=gemsim

all: $(PROG)
	@echo done

$(PROG): Event.h HitEvent.h NeutronEvent.h ReadoutEvent.h Simulator.h Geometry.h main.cpp
	g++ main.cpp -I . --std=c++11 -o $(PROG)

clean:
	rm $(PROG)
