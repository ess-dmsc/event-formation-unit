// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Stream ADC waveforms from file to the EFU.
///
//===----------------------------------------------------------------------===//

#include "DataModulariser.h"
#include "SamplingTimer.h"
#include "UdpConnection.h"
#include "WaveformData.h"

#include <CLI/CLI.hpp>
#include <h5cpp/hdf5.hpp>
#include <string>
#include <vector>

bool RunLoop = true;

void signalHandler(int signal) {
  std::cout << "Got exit signal:" << signal << std::endl;
  RunLoop = false;
}

struct StreamSettings {
  std::string EFUAddress{"localhost"};
  std::uint16_t Port{65535};
  double EventRate{1000};
  std::string NeXuSFile;
  std::string WaveformPath;
};

void addCLIOptions(CLI::App &Parser, StreamSettings &Settings) {
  Parser
      .add_option("--efu_addr", Settings.EFUAddress,
                  "Address to which the data should be transmitted.")
      ->default_str("localhost");
  Parser
      .add_option("--port", Settings.Port,
                  "UDP port to which the data should be transmitted.")
      ->default_str("65535");
  Parser
      .add_option("--event_rate", Settings.EventRate,
                  "Events (waveforms) transmitted per second.")
      ->default_str("1000");
  Parser
      .add_option("--data_file", Settings.NeXuSFile,
                  "Path to (NeXus) file to be used as a data source.")
      ->required(true);
  Parser
      .add_option("--nexus_path", Settings.WaveformPath,
                  "Path to group that has the waveform data. E.g. "
                  "\"/entry/waveform_channel_\"")
      ->required(true);
}

class FileSampler {
public:
  FileSampler(std::string const &FileName, std::string const &NexusPath)
      : InFile(hdf5::file::open(FileName, hdf5::file::AccessFlags::READONLY)) {
    RootNode = InFile.root();
    for (int i = 0; i < 4; i++) {
      auto Group = RootNode.get_group(NexusPath + std::to_string(i));
      Channels.emplace_back(Group);
    }
    for (auto &Ch : Channels) {
      CurrentTimestamp.emplace_back(
          std::make_pair(CurrentTimestamp.size(), Ch.getTimestamp()));
    }
  }
  std::pair<void const *const, std::size_t> generate() {
    auto MinimumTs = std::min_element(
        CurrentTimestamp.begin(), CurrentTimestamp.end(),
        [](auto const &A, auto const &B) { return A.second < B.second; });
    auto &CurrentChannel = Channels.at(MinimumTs->first);
    if (CurrentChannel.outOfData()) {
      RunLoop = false;
      return {nullptr, 0};
    }
    auto TempPair =
        Modulariser.modularise(CurrentChannel.getWaveform(),
                               CurrentChannel.getTimestamp(), MinimumTs->first);
    CurrentChannel.nextWaveform();
    MinimumTs->second = CurrentChannel.getTimestamp();
    return TempPair;
  }

private:
  hdf5::file::File InFile;
  hdf5::node::Group RootNode;
  std::vector<WaveformData> Channels;
  std::vector<std::pair<int, std::uint64_t>> CurrentTimestamp;
  DataModulariser Modulariser;
};

struct PoissionFileGenerator {
  PoissonDelayData PoissonData;
  FileSampler FS;

  TimeDurationNano calcDelaTime() {
    double DelayTime =
        PoissonData.RandomDistribution(PoissonData.RandomGenerator);
    TimeDurationNano NextEventDelay(static_cast<size_t>(DelayTime * 1e9));
    return NextEventDelay;
  }

  void genSamplesAndQueueSend(const TimeStamp &Time) {
    std::pair<void const *const, std::size_t> SampleRun = FS.generate();
    PoissonData.TimerData.UdpCon->addSamplingRun(SampleRun.first,
                                                 SampleRun.second, Time);
  }
};

PoissionFileGenerator CreatePoissionFileGenerator(UdpConnection *UdpCon,
                                                  StreamSettings &Settings) {
  double Rate = Settings.EventRate;

  SampleRunGenerator Dummy(0, 0, 0, 0, 0, 0, 0);

  SamplingTimerData TimerData{
      SamplerType::PoissonDelay, UdpCon, Dummy, 0, 0, Rate};

  std::random_device RandomDevice;
  PoissonDelayData data = {TimerData,
                           std::default_random_engine(RandomDevice()),
                           std::exponential_distribution<double>(Rate)};

  FileSampler FS(Settings.NeXuSFile, Settings.WaveformPath);

  return PoissionFileGenerator{data, std::move(FS)};
}

int main(const int argc, char *argv[]) {
  StreamSettings UsedSettings;
  CLI::App CLIParser{"Adc file streamer"};
  addCLIOptions(CLIParser, UsedSettings);
  try {
    CLIParser.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    CLIParser.exit(e);
    return 0;
  }

  UdpConnection UdpCon(UsedSettings.EFUAddress, UsedSettings.Port);
  TimePointNano TriggerTime_UdpHeartBeat;

  PoissionFileGenerator PoissionFile =
      CreatePoissionFileGenerator(&UdpCon, UsedSettings);
  TimePointNano TriggerTime_PoissonFile;

  auto PrintStats = [&UdpCon]() {
    std::cout << "--------------------------\n";
    std::cout << "Sampling runs:   " << UdpCon.getNrOfRuns() << "\n";
    std::cout << "Created packets: " << UdpCon.getNrOfPackets() << "\n";
    std::cout << "Sent packets:    " << UdpCon.getNrOfSentPackets() << "\n";
  };
  TimeDurationNano PrintStatsInterval(5'000'000'000ull);
  TimePointNano TriggerTime_PrintStats;

  bool runTriggers = false;
  while (RunLoop) {

    auto TimeNow = std::chrono::high_resolution_clock::now();
    TimeStamp TimeTS = MakeExternalTimeStampFromClock(TimeNow);

    // PoissionFile
    {
      auto &Self = PoissionFile;
      auto &TriggerTime = TriggerTime_PoissonFile;
      if (TriggerTime <= TimeNow) {
        TriggerTime = TimeNow + Self.calcDelaTime();
        if (runTriggers) {
          Self.genSamplesAndQueueSend(TimeTS);
        }
      }
    }

    // PrintStats
    {
      auto &TriggerTime = TriggerTime_PrintStats;
      if (TriggerTime <= TimeNow) {
        TriggerTime = TimeNow + PrintStatsInterval;
        if (runTriggers) {
          PrintStats();
        }
      }
    }

    // UdpConnection HeatBeat
    {
      auto &Self = UdpCon;
      auto &TriggerTime = TriggerTime_UdpHeartBeat;
      if (TriggerTime <= TimeNow) {
        TriggerTime = TimeNow + Self.HeartbeatInterval;
        if (runTriggers) {
          Self.transmitHeartbeat();
        }
      }
    }

    // UdpConnection FlushIdleData
    {
      auto &Self = UdpCon;
      if (Self.shouldFlushIdleDataPacket(TimeNow)) {
        if (runTriggers) {
          Self.flushIdleDataPacket(TimeNow);
        }
      }
    }

    runTriggers = true;
  }

  std::cout << "\nWaiting for transmit thread to exit!" << std::endl;
  UdpCon.waitTransmitDone();

  std::cout << "\nFinal tally:\n";
  std::cout << "Sampling runs:   " << UdpCon.getNrOfRuns() << "\n";
  std::cout << "Created packets: " << UdpCon.getNrOfPackets() << "\n";
  std::cout << "Sent packets:    " << UdpCon.getNrOfSentPackets() << "\n";
  return 0;
}
