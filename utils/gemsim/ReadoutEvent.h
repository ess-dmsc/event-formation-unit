/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Derived class for Readout Events
 */

#pragma once
#include <Event.h>

class ReadoutEvent : public Event {
public:
  static bool debug;

  ReadoutEvent(int fec, int asic, int channel, int adc, double t)
     : Event(t), fec_(fec), asic_(asic), channel_(channel), adc_(adc)  { }

  void execute(Simulator * sim) {
    printf("%.10f readout fec %2d, asic: %1d, channel %2d, adc %5d\n",
        time, fec_, asic_, channel_, adc_);
  }
private:
  int fec_{0};
  int asic_{0};
  int channel_{0};
  int adc_{0};
};

bool ReadoutEvent::debug = false;
