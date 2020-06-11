#include "AmpEventDelay.h"
#include <random>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

std::random_device RandomDevice;
std::default_random_engine Generator(RandomDevice());
std::uniform_real_distribution<double> Distribution(0, 3.141592653);

auto generateCircleAmplitudes() {
  const double Amplitude{2000};
  const double Center{3000};
  auto Angle = Distribution(Generator);
  return std::make_pair(Center + Amplitude * std::cos(Angle),
                        Center + Amplitude * std::sin(Angle));
}

void AmpEventDelay::runFunction() {
  auto Now = system_clock::now();
  auto NowSeconds = duration_cast<seconds>(Now.time_since_epoch()).count();
  double NowSecFrac =
      (duration_cast<nanoseconds>(Now.time_since_epoch()).count() / 1e9) -
      NowSeconds;
  std::uint32_t Ticks = std::lround(NowSecFrac * (88052500 / 2.0));

  RawTimeStamp rts{static_cast<uint32_t>(NowSeconds), Ticks};
  TimeStamp Time(rts, TimeStamp::ClockMode::External);

  ////////////////

  auto SampleRunAnode = data.AnodeGen.generate(2000.0, Time);
  auto Amplitudes = generateCircleAmplitudes();
  auto SampleRunX = data.XPosGen.generate(Amplitudes.first, Time);
  auto SampleRunY = data.YPosGen.generate(Amplitudes.second, Time);
  data.sgd.FPGAPtr->addSamplingRun(SampleRunAnode.first, SampleRunAnode.second,
                              Time);
  data.sgd.FPGAPtr->addSamplingRun(SampleRunX.first, SampleRunX.second, Time);
  data.sgd.FPGAPtr->addSamplingRun(SampleRunY.first, SampleRunY.second, Time);
}