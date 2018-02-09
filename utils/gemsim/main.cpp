#include <cstdio>
#include <cstdlib>
#include <Simulator.h>
#include <NeutronEvent.h>

int main(int argc, char * argv[])
{
  Simulator gemsim;
  NeutronEvent::debug = true;
  ReadoutEvent::debug = true;
  HitEvent::debug = true;


  int strips_per_neutron = 3;
  int neutron_events = 100;
  double t = 0;

  if (argc == 3) {
    neutron_events = atoi(argv[1]);
    strips_per_neutron = atoi(argv[2]);
  }

  for (int i = 0; i < neutron_events; i++) {
    auto x = gemsim.geom.getRandomX();
    auto y = gemsim.geom.getRandomY();
    gemsim.addEvent(new NeutronEvent(strips_per_neutron, x, y, t));
      t += 10 * us; // 10us
  }

  // sim.addEvent(new NeutronEvent(0.0, 0.0, 0.7));
  // sim.addEvent(new HitEvent(1, 1000, 1.0));

  gemsim.run();

  return 0;
}
