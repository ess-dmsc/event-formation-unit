#include <cstdio>
#include <cstdlib>
#include <Event.h>
#include <HitEvent.h>
#include <Simulator.h>
#include <NeutronEvent.h>

int main(int argc, char * argv[])
{
  Simulator sim;
  int strips_per_neutron = 5;
  int neutron_events = 10000;
  double t = 0;


  for (int i = 0; i < neutron_events; i++) {
    auto x = sim.geom.getRandomX();
    auto y = sim.geom.getRandomY();
    //printf("x,y: %f, %f\n", x, y);
    sim.addEvent(new NeutronEvent(strips_per_neutron, x, y, t));
    //t += 0.000001; // 1us
    t += 0.00001; // 10s
  }

  // sim.addEvent(new NeutronEvent(0.0, 0.0, 0.7));
  // sim.addEvent(new HitEvent(1, 1000, 1.0));

  sim.doAllEvents();

  return 0;
}
