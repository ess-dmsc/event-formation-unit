/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Simple application for simulating chopper TDC and monitor event data
 * production.
 */

#include "AmpEventDelay.h"
#include "ContinousSamplingTimer.h"
#include "FPGASim.h"
#include "PoissonDelay.h"
#include "SampleRunGenerator.h"
#include <CLI/CLI.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>

bool RunLoop = true;

void signalHandler(int signal) {
  std::cout << "Got exit signal:" << signal << std::endl;
  RunLoop = false;
}

enum class RunMode { SIMPLE, DELAY_LINE_TIME, DELAY_LINE_AMP, CONTINOUS };

struct SimSettings {
  std::string EFUAddress{"localhost"};
  std::uint16_t Port1{65535};
  std::uint16_t Port2{65534};
  bool SecondFPGA{false};
  RunMode Mode{RunMode::SIMPLE};
  double EventRate{1000};
  double NoiseRate{0.5};
};

void addCLIOptions(CLI::App &Parser, SimSettings &Settings) {
  Parser
      .add_option("--efu_addr", Settings.EFUAddress,
                  "Address to which the data should be transmitted.")
      ->default_str("localhost");
  Parser
      .add_option(
          "--port1", Settings.Port1,
          "UDP port #1 (from FPGA 1) to which the data should be transmitted.")
      ->default_str("65535");
  Parser
      .add_option(
          "--port2", Settings.Port1,
          "UDP port #2 (from FPGA 2) to which the data should be transmitted.")
      ->default_str("65534");
  Parser.add_flag("--two_fpgas", Settings.SecondFPGA,
                  "Simulate a second FPGA (data source).");
  std::vector<std::pair<std::string, RunMode>> ModeMap{
      {"simple", RunMode::SIMPLE},
      {"delay_line_time", RunMode::DELAY_LINE_TIME},
      {"delay_line_amp", RunMode::DELAY_LINE_AMP},
      {"continous", RunMode::CONTINOUS},
  };
  Parser
      .add_option("--mode", Settings.Mode,
                  "Set the simulation (data generation) mode.")
      ->transform(CLI::CheckedTransformer(ModeMap, CLI::ignore_case))
      ->default_str("continous");

  Parser
      .add_option("--event_rate", Settings.EventRate,
                  "Event rate when mode is set to delay-line time or "
                  "delay-line amplitude.")
      ->default_str("1000");

  Parser
      .add_option(
          "--noise_rate", Settings.NoiseRate,
          "Noise rate per channel. Applies to all modes except continous.")
      ->default_str("0.5");
}

using namespace std::chrono_literals;

auto SetUpNoiseGenerator(asio::io_service &Service, FPGASim *FPGAPtr, int BoxNr,
                         int ChNr, std::map<std::string, double> Settings) {
  // std::shared_ptr<SampleRunGenerator> SampleGen =
  //    std::make_shared<SampleRunGenerator>(100, 50, 20, 1.0,
  //                                         Settings.at("offset"), BoxNr,
  //                                         ChNr);

  double Settings_offset = Settings.at("offset");
  double Settings_amplitude = Settings.at("amplitude");
  double Settings_rate = Settings.at("rate");

  SampleRunGenerator SampleGen(100, 50, 20, 1.0, Settings_offset, BoxNr, ChNr);

  PoissonDelayData sgd = {&Service,           FPGAPtr,
                        SampleGen,          Settings_offset,
                        Settings_amplitude, Settings_rate};
  /*
    struct NoiseGenSampleRunData{
      std::map<std::string, double> Settings;
      std::shared_ptr<SampleRunGenerator> SampleGen;
      FPGASim* FPGAPtr;

        std::shared_ptr<SampleRunGenerator> SampleGen =
        std::make_shared<SampleRunGenerator>(100, 50, 20, 1.0,
                                             Settings.at("offset"), BoxNr,
    ChNr);

      void samplingFunc(TimeStamp const &Time) {
        std::pair<void *, std::size_t> SampleRun =
            SampleGen->generate(Settings.at("amplitude"), Time);
        FPGAPtr->addSamplingRun(SampleRun.first, SampleRun.second, Time);
      }
    };
    */

  // auto Glue = [Settings, SampleGen, FPGAPtr](TimeStamp const &Time) {
  //  auto SampleRun = SampleGen->generate(Settings.at("amplitude"), Time);
  //  FPGAPtr->addSamplingRun(SampleRun.first, SampleRun.second, Time);
  //};

  //     std::make_shared<PoissonDelay>(Glue, Service, Settings.at("rate"));

  return std::make_shared<PoissonDelay>(sgd);
}

auto SetUpContGenerator(asio::io_service &Service, FPGASim *FPGAPtr, int BoxNr,
                        int ChNr, std::map<std::string, double> Settings) {
  const int NrOfSamples = 4468;
  const int OversamplingFactor = 4;
  auto SampleGen = std::make_shared<SampleRunGenerator>(
      NrOfSamples, 50, 20, 1.0, Settings.at("offset"), BoxNr, ChNr);
  auto Glue = [Settings, SampleGen, FPGAPtr](TimeStamp const &Time) {
    auto SampleRun = SampleGen->generate(Settings.at("amplitude"), Time);
    FPGAPtr->addSamplingRun(SampleRun.first, SampleRun.second, Time);
  };
  return std::make_shared<ContinousSamplingTimer>(Glue, Service, NrOfSamples,
                                                  OversamplingFactor);
}


auto SetUpAmpPosGenerator(asio::io_service &Service, FPGASim *FPGAPtr,
                          int BoxNr, double EventRate) {

  const int NrOfSamples{100};

  double Settings_offset = 0.0;
  double Settings_amplitude = 0.0;
  double Settings_rate = EventRate;
  SampleRunGenerator SampleGen(NrOfSamples, 50, 20, 1.0, Settings_offset, 0, 0);

  PoissonDelayData sgd = {&Service,           FPGAPtr,
                        SampleGen,          Settings_offset,
                        Settings_amplitude, Settings_rate};

  SampleRunGenerator AnodeGen(NrOfSamples, 50, 20, 1.0, 500, BoxNr, 0);
  SampleRunGenerator XPosGen(NrOfSamples, 50, 20, 1.0, 500, BoxNr, 1);
  SampleRunGenerator YPosGen(NrOfSamples, 50, 20, 1.0, 500, BoxNr, 2);

  AmpEventDelayData agd = {AnodeGen, XPosGen, YPosGen, sgd};

  return std::make_shared<AmpEventDelay>(agd);
}

class StatsTimer {
public:
  StatsTimer(std::function<void()> OnTimer, asio::io_service &Service)
      : TimerFunc(std::move(OnTimer)), Timer(Service) {}

  void start() {
    Timer.expires_after(5s);
    Timer.async_wait([this](auto &Error) {
      if (not Error) {
        TimerFunc();
        start();
      };
    });
  }
  void stop() { Timer.cancel(); }

private:
  std::function<void()> TimerFunc;
  asio::system_timer Timer;
};

int main(const int argc, char *argv[]) {
  SimSettings UsedSettings;
  CLI::App CLIParser{"Detector simulator"};
  addCLIOptions(CLIParser, UsedSettings);
  try {
    CLIParser.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    CLIParser.exit(e);
    return 0;
  }
  const std::vector<int> ActiveChannels{0, 1, 2, 3};

  std::signal(SIGINT, signalHandler);

  asio::io_service Service;
  asio::io_service::work Worker(Service);

  auto AdcBox1 = std::make_shared<FPGASim>(UsedSettings.EFUAddress,
                                           UsedSettings.Port1, Service);
  auto AdcBox2 = std::make_shared<FPGASim>(UsedSettings.EFUAddress,
                                           UsedSettings.Port2, Service);

  std::vector<std::shared_ptr<SamplingTimer>> Box1Timers;

  {
    auto Temp1 = SetUpNoiseGenerator(Service, AdcBox1.get(), 0, 0,
                                     {{"rate", UsedSettings.NoiseRate},
                                      {"offset", 1.0},
                                      {"amplitude", 2000.0}});
    Box1Timers.emplace_back(Temp1);

    auto Temp2 = SetUpNoiseGenerator(Service, AdcBox1.get(), 0, 1,
                                     {{"rate", UsedSettings.NoiseRate},
                                      {"offset", 10.0},
                                      {"amplitude", 3000.0}});
    Box1Timers.emplace_back(Temp2);

    auto Temp3 = SetUpNoiseGenerator(Service, AdcBox1.get(), 0, 2,
                                     {{"rate", UsedSettings.NoiseRate},
                                      {"offset", 100.0},
                                      {"amplitude", 4000.0}});
    Box1Timers.emplace_back(Temp3);

    auto Temp4 = SetUpNoiseGenerator(Service, AdcBox1.get(), 0, 3,
                                     {{"rate", UsedSettings.NoiseRate},
                                      {"offset", 1000.0},
                                      {"amplitude", 5000.0}});
    Box1Timers.emplace_back(Temp4);

    auto Temp5 =
        SetUpContGenerator(Service, AdcBox1.get(), 0, 0,
                           {{"offset", 1000.0}, {"amplitude", 1000.0}});
    Box1Timers.emplace_back(Temp5);

    auto Temp6 =
        SetUpAmpPosGenerator(Service, AdcBox1.get(), 0, UsedSettings.EventRate);
    Box1Timers.emplace_back(Temp6);
  }

  std::vector<std::shared_ptr<SamplingTimer>> Box2Timers;

  {
    auto Temp1 = SetUpNoiseGenerator(Service, AdcBox2.get(), 1, 0,
                                     {{"rate", UsedSettings.NoiseRate},
                                      {"offset", 1.0},
                                      {"amplitude", 2000.0}});
    Box2Timers.emplace_back(Temp1);

    auto Temp2 = SetUpNoiseGenerator(Service, AdcBox2.get(), 1, 1,
                                     {{"rate", UsedSettings.NoiseRate},
                                      {"offset", 10.0},
                                      {"amplitude", 3000.0}});
    Box2Timers.emplace_back(Temp2);

    auto Temp3 = SetUpNoiseGenerator(Service, AdcBox2.get(), 1, 2,
                                     {{"rate", UsedSettings.NoiseRate},
                                      {"offset", 100.0},
                                      {"amplitude", 4000.0}});
    Box2Timers.emplace_back(Temp3);

    auto Temp4 = SetUpNoiseGenerator(Service, AdcBox2.get(), 1, 3,
                                     {{"rate", UsedSettings.NoiseRate},
                                      {"offset", 1000.0},
                                      {"amplitude", 5000.0}});
    Box2Timers.emplace_back(Temp4);

    auto Temp5 = SetUpContGenerator(Service, AdcBox2.get(), 1, 0,
                                    {{"offset", 1000.0}, {"amplitude", 500.0}});
    Box2Timers.emplace_back(Temp5);
  }
  auto UsedRunMode = UsedSettings.Mode;
  auto ActivateTimers = [UsedRunMode](auto &TimerList) {
    for (auto &Timer : TimerList) {
      bool StartTimer = false;
      if (UsedRunMode == RunMode::SIMPLE or
          UsedRunMode == RunMode::DELAY_LINE_AMP or
          UsedRunMode == RunMode::DELAY_LINE_TIME) {
        if (dynamic_cast<PoissonDelay *>(Timer.get()) != nullptr and
            dynamic_cast<AmpEventDelay *>(Timer.get()) == nullptr) {
          StartTimer = true;
        }
      }
      if (UsedRunMode == RunMode::DELAY_LINE_AMP and
          dynamic_cast<AmpEventDelay *>(Timer.get()) != nullptr) {
        StartTimer = true;
      }
      if (UsedRunMode == RunMode::CONTINOUS) {
        if (dynamic_cast<ContinousSamplingTimer *>(Timer.get()) != nullptr) {
          StartTimer = true;
        }
      }

      if (StartTimer) {
        Timer->start();
      }
    }
  };

  ActivateTimers(Box1Timers);

  if (UsedSettings.SecondFPGA) {
    ActivateTimers(Box2Timers);
  }
  auto PrintStats = [&AdcBox1, &AdcBox2]() {
    std::cout << "Sampling runs generated by FPGA simulator 1: "
              << AdcBox1->getNrOfRuns();
    std::cout << "\nSampling runs generated by FPGA simulator 2: "
              << AdcBox2->getNrOfRuns() << "\n";
  };

  StatsTimer Stats(PrintStats, Service);
  Stats.start();

  //std::thread AsioThread([&Service]() { Service.run(); });
  std::thread AsioThread([]{});
  Service.run();

  while (RunLoop) {
    std::this_thread::sleep_for(500ms);
  }

  std::cout << "Waiting for transmit thread to exit!" << std::endl;
  Service.stop();
  if (AsioThread.joinable()) {
    AsioThread.join();
  }
  return 0;
}
