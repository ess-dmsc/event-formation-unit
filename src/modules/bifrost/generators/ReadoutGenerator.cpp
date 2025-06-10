// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of the ReadoutGenerator class
///
//===----------------------------------------------------------------------===//

// GCOVR_EXCL_START

#include <bifrost/generators/ReadoutGenerator.h>
#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <fcntl.h>
#include <sys/stat.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

ReadoutGenerator::ReadoutGenerator()
    : ReadoutGeneratorBase(DetectorType::BIFROST) {
  app.add_option("-n, --data", bifrostSettings.FilePath,
                 "Record data file to read from");
}

int ReadoutGenerator::readReadout(struct dat_data_t &Readout) {

  if (FileDescriptor == -1) {
    FileDescriptor = open(bifrostSettings.FilePath.c_str(), O_RDONLY);
  }

  if (FileDescriptor == -1) {
    throw std::runtime_error(
        "Failed to open the file: " + bifrostSettings.FilePath +
        ". Biforst gen requires dat file with sample data");
  }

  int res = read(FileDescriptor, (void *)&Readout, sizeof(struct dat_data_t));

  if (res == sizeof(struct dat_data_t)) {

    XTRACE(DATA, WAR,
           "ring %2u, fen 0, time (%5u, %7u) - tube %2u, A: %5u, B: %5u\n",
           Readout.fiber, Readout.timehi, Readout.timelow, Readout.tube,
           Readout.ampl_a, Readout.ampl_b);

    return 1;
  }

  XTRACE(DATA, DEB, "Reached EOF, restart from beginning");
  // File is read to the end, restart from the beginning
  lseek(FileDescriptor, 0, SEEK_SET);
  return -1;
}

void ReadoutGenerator::generateData() {

  uint32_t SentReadouts = 0;

  struct dat_data_t DatReadout;
  /// \todo: instead setting dataPkt in memory and copying it to the buffer, set
  /// it directly in the buffer
  struct Caen::DataParser::CaenReadout dataPkt;

  memset(&dataPkt, 0, sizeof(dataPkt));

  int res = 0;

  auto dataPtr = (uint8_t *)Buffer;
  dataPtr += HeaderSize;

  while (((res = readReadout(DatReadout)) > 0) &&
         (SentReadouts < ReadoutPerPacket)) {

    dataPkt.FiberId = DatReadout.fiber;
    dataPkt.FENId = 0;
    dataPkt.DataLength = ReadoutDataSize;

    auto [readoutTimeHigh, readoutTimeLow] = generateReadoutTime();
    dataPkt.TimeHigh = readoutTimeHigh;
    dataPkt.TimeLow = readoutTimeLow;
    dataPkt.Group = DatReadout.tube;
    dataPkt.AmpA = DatReadout.ampl_a;
    dataPkt.AmpB = DatReadout.ampl_b;

    memcpy(dataPtr, &dataPkt, ReadoutDataSize);
    dataPtr += ReadoutDataSize;

    SentReadouts++;
  }
}

void ReadoutGenerator::main() {
  DistributionGenerator distribution(Settings.Frequency);
  ReadoutGeneratorBase::initialize(&distribution);

  // If the number of packets is not set, calculate it how much packet is
  // required to send all the readouts in the file
  if (Settings.NumberOfPackets == 0 && Settings.Loop == false) {
    struct stat fileStat;

    if (stat(bifrostSettings.FilePath.c_str(), &fileStat) == -1) {
      throw std::runtime_error(
          "Failed to open the file: " + bifrostSettings.FilePath +
          ". Biforst gen requires dat file with sample data");
    }

    off_t fileSize = fileStat.st_size;

    // Calculate the number of dat_data_t that can fit into the file size
    size_t readoutInDatFile = fileSize / sizeof(struct dat_data_t);

    Settings.NumberOfPackets = readoutInDatFile / ReadoutPerPacket;
  }
}

// GCOVR_EXCL_STOP
