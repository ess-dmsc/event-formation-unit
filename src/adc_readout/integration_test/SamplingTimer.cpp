/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "SamplingTimer.h"

#ifndef CONTINOUS_SAMPLE_GENERATOR_HIGH_OVERHEAD_TEST
#define CONTINOUS_SAMPLE_GENERATOR_HIGH_OVERHEAD_TEST 0
#endif

TimeDurationNano PoissonDelay::calcDelaTime() {
  double DelayTime = Data.RandomDistribution(Data.RandomGenerator);
  TimeDurationNano NextEventDelay(static_cast<size_t>(DelayTime * 1e9));
  return NextEventDelay;
}

void PoissonDelay::genSamplesAndQueueSend(const TimeStamp &Time) {
  std::pair<void *, std::size_t> SampleRun =
      Data.TimerData.SampleGen.generate(Data.TimerData.Amplitude, Time);
  Data.TimerData.UdpCon->addSamplingRun(SampleRun.first, SampleRun.second,
                                        Time);
}

PoissonDelay PoissonDelay::Create(UdpConnection *UdpCon, int BoxNr, int ChNr,
                                  std::map<std::string, double> Settings) {
  double Offset = Settings.at("offset");
  double Amplitude = Settings.at("amplitude");
  double Rate = Settings.at("rate");

  SampleRunGenerator SampleGen(100, 50, 20, 1.0, Offset, BoxNr, ChNr);

  SamplingTimerData TimerData{
      SamplerType::PoissonDelay, UdpCon, SampleGen, Offset, Amplitude, Rate};

  std::random_device RandomDevice;
  PoissonDelayData Data = {TimerData,
                           std::default_random_engine(RandomDevice()),
                           std::exponential_distribution<double>(Rate)};

  return PoissonDelay{Data};
}

//-----------------------------------------------------------------------------

static std::random_device AmpEventDelay_RandomDevice;
static std::default_random_engine
    AmpEventDelay_Generator(AmpEventDelay_RandomDevice());
static std::uniform_real_distribution<double>
    AmpEventDelay_DistributionPi(0, 3.141592653);

auto generateCircleAmplitudes() {
  const double Amplitude{2000};
  const double Center{3000};
  auto Angle = AmpEventDelay_DistributionPi(AmpEventDelay_Generator);
  return std::make_pair(Center + Amplitude * std::cos(Angle),
                        Center + Amplitude * std::sin(Angle));
}

TimeDurationNano AmpEventDelay::calcDelaTime() {
  double DelayTime =
      Data.PoissonData.RandomDistribution(Data.PoissonData.RandomGenerator);
  TimeDurationNano NextEventDelay(static_cast<size_t>(DelayTime * 1e9));
  return NextEventDelay;
}

void AmpEventDelay::genSamplesAndQueueSend(const TimeStamp &Time) {

  auto SampleRunAnode = Data.AnodeGen.generate(2000.0, Time);
  auto Amplitudes = generateCircleAmplitudes();
  auto SampleRunX = Data.XPosGen.generate(Amplitudes.first, Time);
  auto SampleRunY = Data.YPosGen.generate(Amplitudes.second, Time);
  Data.PoissonData.TimerData.UdpCon->addSamplingRun(
      SampleRunAnode.first, SampleRunAnode.second, Time);
  Data.PoissonData.TimerData.UdpCon->addSamplingRun(SampleRunX.first,
                                                    SampleRunX.second, Time);
  Data.PoissonData.TimerData.UdpCon->addSamplingRun(SampleRunY.first,
                                                    SampleRunY.second, Time);
}

AmpEventDelay AmpEventDelay::Create(UdpConnection *UdpCon, int BoxNr,
                                    double EventRate) {

  const int NrOfSamples{100};

  double Offset = 0.0;
  double Amplitude = 0.0;
  double Rate = EventRate;
  SampleRunGenerator SampleGen(NrOfSamples, 50, 20, 1.0, Offset, 0, 0);

  SamplingTimerData TimerData = {
      SamplerType::AmpEventDelay, UdpCon, SampleGen, Offset, Amplitude, Rate};

  std::random_device RandomDevice;
  PoissonDelayData PoissonData = {TimerData,
                                  std::default_random_engine(RandomDevice()),
                                  std::exponential_distribution<double>(Rate)};

  SampleRunGenerator AnodeGen(NrOfSamples, 50, 20, 1.0, 500, BoxNr, 0);
  SampleRunGenerator XPosGen(NrOfSamples, 50, 20, 1.0, 500, BoxNr, 1);
  SampleRunGenerator YPosGen(NrOfSamples, 50, 20, 1.0, 500, BoxNr, 2);

  AmpEventDelayData Data = {PoissonData, AnodeGen, XPosGen, YPosGen};

  return AmpEventDelay{Data};
}

//-----------------------------------------------------------------------------

TimeDurationNano ContinousSamplingTimer::calcDelaTime() {
  return Data.TimeStepNano;
}

void ContinousSamplingTimer::genSamplesAndQueueSend(const TimeStamp &Time) {
  std::pair<void *, std::size_t> SampleRun =
      Data.TimerData.SampleGen.generate(Data.TimerData.Amplitude, Time);
  Data.TimerData.UdpCon->addSamplingRun(SampleRun.first, SampleRun.second,
                                        Time);
}

ContinousSamplingTimer
ContinousSamplingTimer::Create(UdpConnection *UdpCon, int BoxNr, int ChNr,
                               std::map<std::string, double> Settings) {

  double Offset = Settings.at("offset");
  double Amplitude = Settings.at("amplitude");
  double Rate = 0;

  int NrOfSamples = 4468;
  int OversamplingFactor = 4;
  if (CONTINOUS_SAMPLE_GENERATOR_HIGH_OVERHEAD_TEST) {
    NrOfSamples = 100;
    OversamplingFactor = 1;
  }
  int NrOfOriginalSamples = NrOfSamples * OversamplingFactor;

  SampleRunGenerator SampleGen(NrOfSamples, 50, 20, 1.0, Offset, BoxNr, ChNr);

  const double TimeFracMax = TimerClockFrequencyExternal / 2.0;
  TimeDurationNano TimeStepNano(
      static_cast<std::uint64_t>((NrOfOriginalSamples / TimeFracMax) * 1e9));

  SamplingTimerData TimerData{
      SamplerType::Continous, UdpCon, SampleGen, Offset, Amplitude, Rate};

  ContinousSamplingTimerData Data = {TimerData, TimeStepNano};

  return ContinousSamplingTimer{Data};
}
