/** Copyright (C) 2016-2020 European Spallation Source ERIC */
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementation of Class for command line options
///
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <common/Version.h>
#include <common/debug/Log.h>
#include <common/detector/EFUArgs.h>
#include <cstdio>
#include <fstream>
#include <regex>
#include <string>

using namespace std::literals::string_literals;

EFUArgs::EFUArgs() {
  // clang-format off
  CLIParser.set_help_flag(); // Removes the default help flag
  //CLIParser.allow_extras(false);
  CLIParser.get_formatter()->column_width(30);

  HelpOption = CLIParser.add_flag("-h,--help", "Print this help message and exit")
      ->group("EFU Options")->configurable(false);

  CLIParser.add_flag("--version", PrintVersion, "Print version and exit")
      ->group("EFU Options")->configurable(false);

  CLIParser.add_option("-a,--logip", GraylogConfig.address, "Graylog server IP address")
      ->group("EFU Options")->default_str("127.0.0.1");

  CLIParser.add_option("-b,--broker_addr", EFUSettings.KafkaBroker, "Kafka broker address")
      ->group("EFU Options")->default_str("localhost");

  CLIParser.add_option("-t,--broker_topic", EFUSettings.KafkaTopic, "Kafka broker topic")
      ->group("EFU Options")->default_str("");

  CLIParser.add_option("--kafka_config", EFUSettings.KafkaConfigFile, "Kafka configuration file")
      ->group("EFU Options")->default_str("");

  CLIParser.add_option("-l,--log_level", [this](std::vector<std::string> Input) {
    return parseLogLevel(Input);
  }, "Set log message level. Set to 1 - 7 or one of \n                              `Critical`, `Error`, `Warning`, `Notice`, `Info`,\n                              or `Debug`. Ex: \"-l Notice\"")
  ->group("EFU Options")->default_str("Info");

  CLIParser.add_option("--log_file", LogFileName, "Write log messages to file.")
  ->group("EFU Options");

  CLIParser.add_flag("--nohwcheck", EFUSettings.NoHwCheck, "Perform HW check or not")
      ->group("EFU Options");

  CLIParser.add_flag("--udder", EFUSettings.TestImage, "Generate a test image")
      ->group("EFU Options");

  CLIParser.add_option("--udder_usleep", EFUSettings.TestImageUSleep, "usleep between udder pixels")
      ->group("EFU Options")->default_str("10");

  std::string DetectorDescription{"Detector name"};

  CLIParser.add_option("-i,--dip", EFUSettings.DetectorAddress,
                       "IP address of receive interface")
      ->group("EFU Options")->default_str("0.0.0.0");

  CLIParser.add_option("-p,--port", EFUSettings.DetectorPort, "TCP/UDP receive port")
      ->group("EFU Options")->default_str("9000");

  CLIParser.add_option("-m,--cmdport", EFUSettings.CommandServerPort,
                       "Command parser tcp port")
      ->group("EFU Options")->default_str("8888");

  CLIParser.add_option("-g,--graphite", EFUSettings.GraphiteAddress,
                       "IP address of graphite metrics server")
      ->group("EFU Options")->default_str("127.0.0.1");

  CLIParser.add_option("-r,--region", EFUSettings.GraphiteRegion,
                       "name of detector region covered by this pipeline")
      ->group("EFU Options")->default_str("region1");

  CLIParser.add_option("-o,--gport", EFUSettings.GraphitePort, "Graphite tcp port")
      ->group("EFU Options")->default_str("2003");

  CLIParser.add_option("-s,--stopafter", EFUSettings.StopAfterSec,
                       "Terminate after timeout seconds")
      ->group("EFU Options")->default_str("4294967295"); // 0xffffffffU

  CLIParser.add_option("--updateinterval", EFUSettings.UpdateIntervalSec,
                       "Stats and event data update interval (seconds).")
      ->group("EFU Options")->default_str("1");

  CLIParser.add_option("--rxbuffer", EFUSettings.RxSocketBufferSize,
                       "Receive from detector buffer size.")
      ->group("EFU Options")->default_str("2000000");

  CLIParser.add_option("--txbuffer", EFUSettings.TxSocketBufferSize,
                  "Transmit to detector buffer size.")
      ->group("EFU Options")->default_str("9216");

  //
  CLIParser.add_option("-f,--file", EFUSettings.ConfigFile,
                  "Detector configuration file (JSON)")
      ->group("EFU Options")->default_str("");

  CLIParser.add_option("--calibration", EFUSettings.CalibFile,
                  "Detector calibration file (JSON)")
      ->group("EFU Options")->default_str("");

  CLIParser.add_option("--dumptofile", EFUSettings.DumpFilePrefix,
                  "dump to specified file")
      ->group("EFU Options")->default_str("");

  // DETECTOR SPECIFIC MULTIGRID
  CLIParser.add_flag("--multigrid-monitor", EFUSettings.MultiGridMonitor,
                "stream monitor data")
      ->group("Multigrid")
      ->configurable(true)
      ->default_val("true");

  // DETECTOR SPECIFIC TTLMONITOR
  CLIParser.add_option("--ttlmonitor-reduce", EFUSettings.TTLMonitorReduceEvents,
          "use 1 out of N readouts")->group("TTLMonitor");

  // EFU LEGACY MODULE
  CLIParser.add_flag("--multiblade-alignment", EFUSettings.MultibladeAlignment,
          "Enter alignment mode (2D)")
          ->group("MBCAEN");

  // clang-format on
}

bool EFUArgs::parseLogLevel(std::vector<std::string> LogLevelString) {
  std::map<std::string, int> LevelMap{{"Critical", 2}, {"Error", 3},
                                      {"Warning", 4},  {"Notice", 5},
                                      {"Info", 6},     {"Debug", 7}};
  if (LogLevelString.size() != 1) {
    return false;
  }
  try {
    LogMessageLevel = LevelMap.at(LogLevelString.at(0));
    return true;
  } catch (std::out_of_range &e) {
    // Do nothing
  }
  try {
    int TempLogMessageLevel = std::stoi(LogLevelString.at(0));
    if (TempLogMessageLevel < 1 or TempLogMessageLevel > 7) {
      return false;
    }
    LogMessageLevel = TempLogMessageLevel;
  } catch (std::invalid_argument &e) {
    return false;
  }
  return true;
}

void EFUArgs::printSettings() {
  // clang-format off
  LOG(INIT, Sev::Info, "Starting event processing pipeline with main properties:");
  LOG(INIT, Sev::Info, "  Detector:                 {}",    DetectorName);
  LOG(INIT, Sev::Info, "  Rx UDP Socket:            {}:{}",
         EFUSettings.DetectorAddress, EFUSettings.DetectorPort);
  LOG(INIT, Sev::Info, "  Perform HW checks         {}", !EFUSettings.NoHwCheck);
  LOG(INIT, Sev::Info, "  Kafka broker:             {}", EFUSettings.KafkaBroker);
  LOG(INIT, Sev::Info, "  Log IP:                   {}", GraylogConfig.address);
  LOG(INIT, Sev::Info, "  Graphite TCP socket:      {}:{}",
        EFUSettings.GraphiteAddress, EFUSettings.GraphitePort);
  LOG(INIT, Sev::Info, "  CLI TCP Socket:           localhost:{}", EFUSettings.CommandServerPort);

  if (EFUSettings.StopAfterSec == 0xffffffffU) {
    LOG(INIT, Sev::Info, "  Stopafter:                never");
  } else {
    LOG(INIT, Sev::Info, "  Stopafter:                {}s", EFUSettings.StopAfterSec);
  }

  LOG(INIT, Sev::Info, "<<< NOT ALL CONFIGURABLE SETTINGS MAY BE DISPLAYED >>>");
  // clang-format on
}

void EFUArgs::printHelp() { std::cout << CLIParser.help(); }

void EFUArgs::printVersion() { std::cout << efu_version() << '\n'; }

EFUArgs::Status EFUArgs::parseArgs(const int argc, char *argv[]) {
  try {
    CLIParser.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    LOG(INIT, Sev::Error, "Invalid CLI argument(s) - error code {}",
        e.get_exit_code());
    return Status::ERREXIT;
  }

  if ((*HelpOption)) {
    printHelp();
    return Status::EXIT;
  }

  if (PrintVersion) {
    printVersion();
    return Status::EXIT;
  }

  return Status::CONTINUE;
}
