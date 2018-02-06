/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Derived class for Neutron Events
 */

#pragma once
#include <cassert>
#include <Geometry.h>
#include <HitEvent.h>

class NeutronEvent : public Event {
public:
  static bool debug;

  NeutronEvent(int nhits, double x, double y, double t)
      : Event(t), n_(nhits), x_(x), y_(y) {};

void execute(Simulator * sim) {
  if (debug) {
    printf("%.10f neutron at (%7.2f, %7.2f)\n", time, x_, y_);
  }

  // Let NeutronEvent generate HitEvents
  auto xstrip = sim->geom.getxStrip(x_);
  auto ystrip = sim->geom.getyStrip(y_);

  for (int i = 0; i < n_; i++) {
    //printf("adding strip %d\n", i);
    auto adc = 1000*(i+1)/n_;
    if (xstrip + i <= 1280) {
      sim->addEvent(new HitEvent(xstrip + i       , adc, time + (i+1)*drifttime));
    }
    if (ystrip + i <= 1280) {
      sim->addEvent(new HitEvent(ystrip + i + 1280, adc, time + (i+1)*drifttime));
    }
  }
}

private:
  int n_{1}; // number of strips hit per neutron
  double x_{0.0};
  double y_{0.0};
  const double drifttime{1.5 * us};
};

bool NeutronEvent::debug = false;
