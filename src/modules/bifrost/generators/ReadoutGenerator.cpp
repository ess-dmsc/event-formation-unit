// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of the ReadoutGenerator class
///
//===----------------------------------------------------------------------===//

#include <bifrost/generators/ReadoutGenerator.h>
#include <common/debug/Trace.h>
#include <fcntl.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <stdexcept>


// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

int ReadoutGenerator::readReadout(struct dat_data_t &Readout) {

  if (FileDescriptor == -1) {
    FileDescriptor = open(Settings.FilePath.c_str(), O_RDONLY);
  }

  if (FileDescriptor == -1) {
    throw std::runtime_error(
        "Failed to open the file: " + Settings.FilePath +
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
  struct Caen::DataParser::CaenReadout dataPkt;

  memset(&dataPkt, 0, sizeof(dataPkt));

  // initialize time low and high for data packets from the header
  if (Settings.headerVersion == ESSReadout::Parser::HeaderVersion::V0) {
    dataPkt.TimeHigh =
        reinterpret_cast<ESSReadout::Parser::PacketHeaderV0 *>(&Buffer)
            ->PulseHigh;
    dataPkt.TimeLow =
        reinterpret_cast<ESSReadout::Parser::PacketHeaderV0 *>(&Buffer)
            ->PulseLow;
  } else if (Settings.headerVersion == ESSReadout::Parser::HeaderVersion::V1) {
    dataPkt.TimeHigh =
        reinterpret_cast<ESSReadout::Parser::PacketHeaderV1 *>(&Buffer)
            ->PulseHigh;
    dataPkt.TimeLow =
        reinterpret_cast<ESSReadout::Parser::PacketHeaderV1 *>(&Buffer)
            ->PulseLow;
  } else {
    throw std::runtime_error("Incorrect header version");
  }

  int res = 0;

  auto dataPtr = (uint8_t *)Buffer;
  dataPtr += HeaderSize;

  while (((res = readReadout(DatReadout)) > 0) &&
         (SentReadouts < Settings.NumReadouts)) {

    dataPkt.FiberId = DatReadout.fiber;
    dataPkt.FENId = 0;
    dataPkt.DataLength = ReadoutDataSize;

    dataPkt.Group = DatReadout.tube;
    dataPkt.AmpA = DatReadout.ampl_a;
    dataPkt.AmpB = DatReadout.ampl_b;

    dataPkt.TimeLow += Settings.TicksBtwReadouts;

    if (dataPkt.TimeLow >= 88052499) {
      dataPkt.TimeLow -= 88052499;
      dataPkt.TimeHigh += 1;
    }

    memcpy(dataPtr, &dataPkt, ReadoutDataSize);
    dataPtr += ReadoutDataSize;

    SentReadouts++;
  }
}
