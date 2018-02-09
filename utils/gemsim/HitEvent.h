/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Derived class for Hit Events
 */


#include <Event.h>
#include <ReadoutEvent.h>
#include <Simulator.h>

class HitEvent : public Event {
public:
  static bool debug;

  HitEvent(int strip, int adc, double t)
     : Event(t), strip_(strip), adc_(adc)  { }

  void execute(Simulator * sim) {
    if (strip_ > 1280) {
      printf("%.10f hit     ystrip %5d, adc %5d\n", time, strip_ - 1280, adc_);
    } else {
      printf("%.10f hit     xstrip %5d, adc %5d\n", time, strip_, adc_);
    }
    auto fec = (strip_ - 1)/128; // x from 0 to 9, y from 10 - 19
    auto channel = (strip_ - fec *128) % 64;
    auto asic = (strip_ - 1 - fec *128)/64;
    assert((fec >= 0) && (fec <= 19));
    assert((channel >=0) && (channel <= 63));
    assert((asic == 0) || (asic == 1));

    sim->addEvent(new ReadoutEvent(fec, asic, channel, adc_, time + readoutlatency)); // fixme
  }
private:
   int strip_{0};
   int adc_{0};
   double readoutlatency = 200 * ns;
};

bool HitEvent::debug = false;
