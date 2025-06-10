// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of the ReadoutGenerator class
///
//===----------------------------------------------------------------------===//

// GCOVR_EXCL_START

#include <dream/generators/g4data/ReadoutGenerator.h>
#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <fcntl.h>
#include <sys/stat.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

ReadoutGenerator::ReadoutGenerator()
    : ReadoutGeneratorBase(DetectorType::DREAM) {
  app.add_option("-n, --data", dreamSettings.FilePath,
                 "Record data file to read from");
}

int ReadoutGenerator::readReadout(struct dat_data_t &Readout) {

  if (FileDescriptor == -1) {
    FileDescriptor = open(dreamSettings.FilePath.c_str(), O_RDONLY);
  }

  if (FileDescriptor == -1) {
    throw std::runtime_error(
        "Failed to open the file: " + dreamSettings.FilePath +
        ". DREAM gen requires dat file with sample data");
  }

  int res = read(FileDescriptor, (void *)&Readout, sizeof(struct dat_data_t));

  if (res == sizeof(struct dat_data_t)) {

    XTRACE(DATA, WAR,
           "timehi %08x, timelo %08x, prevtimehi %08x, prevtimelo %08x\n",
           Readout.pulsetimehi, Readout.pulsetimelo, Readout.prevpulsetimehi);

    XTRACE(DATA, WAR,
           "fiber %2u, fen %2u, timehi %08x, timelo %08x, uid %2u, anode %u, cathode %u\n",
           Readout.fiber, Readout.fen, Readout.timehi, Readout.timelo, Readout.uid,
           Readout.anode, Readout.cathode);

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
  struct Dream::DataParser::CDTReadout dataPkt;

  memset(&dataPkt, 0, sizeof(dataPkt));

  int res = 0;
  ESSReadout::ESSTime time;

  auto dataPtr = (uint8_t *)Buffer;
  dataPtr += HeaderSize;

  while (((res = readReadout(DatReadout)) > 0) &&
         (SentReadouts < ReadoutPerPacket)) {

    dataPkt.FiberId = DatReadout.fiber;
    dataPkt.FENId = DatReadout.fen;
    dataPkt.DataLength = ReadoutDataSize;
    dataPkt.UnitId = DatReadout.uid;
    dataPkt.Cathode = DatReadout.cathode;
    dataPkt.Anode = DatReadout.anode;

    auto [readoutTimeHigh, readoutTimeLow] = generateReadoutTime();
    dataPkt.TimeHigh = readoutTimeHigh;
    dataPkt.TimeLow = readoutTimeLow;

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

    if (stat(dreamSettings.FilePath.c_str(), &fileStat) == -1) {
      throw std::runtime_error(
          "Failed to open the file: " + dreamSettings.FilePath +
          ". DREAM gen requires dat file with sample data");
    }

    off_t fileSize = fileStat.st_size;

    // Calculate the number of dat_data_t that can fit into the file size
    size_t readoutInDatFile = fileSize / sizeof(struct dat_data_t);

    Settings.NumberOfPackets = readoutInDatFile / ReadoutPerPacket;
  }
}

// GCOVR_EXCL_STOP
