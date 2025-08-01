// Copyright (C) 2019 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests
///
//===----------------------------------------------------------------------===//

#include "common/Statistics.h"
#include <generators/functiongenerators/FunctionGenerator.h>
#include <caen/CaenCounters.h>
#include <caen/readout/DataParser.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/TestBase.h>
#include <common/testutils/SocketMock.h>
#include <loki/generators/ReadoutGenerator.h>

#include <memory>
#include <vector>

// Example of UDP readout
// Two Data Sections each containing three readouts
// clang-format off
std::vector<uint8_t> UdpPayload
{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x4e, 0x00, 0x01, 0x00, // len 78/0x4e OQ 1
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, // Seq number 1

    0x00, 0x00, 0x18, 0x00, // Data Header, ring 0, fen 0

    0x00, 0x00, 0x00, 0x00, // Readout 1, time 0
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x02, 0x01,
    0x03, 0x01, 0x04, 0x01,

    0x01, 0x01, 0x18, 0x00, // Data Header 2, ring 1, fen 1

    0x01, 0x00, 0x00, 0x00, // Readout 1, time 1
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x02, 0x01,
    0x03, 0x01, 0x04, 0x01,

};
// clang-format on

using namespace Caen;

class CombinedParserTest : public TestBase {
protected:
  const int DataType{0x30};
  Statistics Stats;
  ESSReadout::Parser ESSHeaderParser{Stats};
  DataParser CaenParser;

  void SetUp() override { ESSHeaderParser.setMaxPulseTimeDiff(4000000000); }
  void TearDown() override {}
};

// Cycle through all section values with equal number of readouts
TEST_F(CombinedParserTest, DataMultiPackage) {
  MultipackageSocketMock socket{};
  const std::chrono::microseconds pulseTimeDuration{71428};
  std::unique_ptr<FunctionGenerator> readoutTimeGenerator =
      std::make_unique<DistributionGenerator>(
          ReadoutGeneratorBase::DEFAULT_FREQUENCY);

  Caen::ReadoutGenerator gen;
  gen.Settings.headerVersion = 0;
  gen.setReadoutDataSize(sizeof(Caen::DataParser::CaenReadout));

  gen.initialize(std::move(readoutTimeGenerator));

  socket.Clear();
  gen.generatePackets(&socket, pulseTimeDuration);

  auto collection = socket.PackageList();
  ASSERT_GT(collection.size(), 1);
  for (auto buffer : collection) {
    int bufferLength = buffer.size();
    ASSERT_GT(bufferLength,
              sizeof(ESSReadout::Parser::PacketHeaderV0) + (4 + 20));
    ASSERT_LT(bufferLength, Caen::ReadoutGenerator::BufferSize);
    auto Res =
        ESSHeaderParser.validate((char *)buffer.data(), bufferLength, DataType);
    ASSERT_EQ(Res, ESSReadout::Parser::OK);
  }
}

// Cycle through all section values with equal number of readouts
TEST_F(CombinedParserTest, DataGenV0) {
  SocketMock socket{};
  const std::chrono::microseconds pulseTimeDuration{0};

  std::unique_ptr<FunctionGenerator> readoutTimeGenerator =
      std::make_unique<DistributionGenerator>(
          ReadoutGeneratorBase::DEFAULT_FREQUENCY);

  Caen::ReadoutGenerator gen;
  gen.Settings.headerVersion = 0;
  gen.setReadoutDataSize(sizeof(Caen::DataParser::CaenReadout));
  gen.initialize(std::move(readoutTimeGenerator));

  for (unsigned int Sections = 1; Sections < 372; Sections++) {

    gen.setReadoutPerPacket(Sections);

    socket.Clear();
    gen.generatePackets(&socket, pulseTimeDuration);

    unsigned long packageSize = socket.GetData().size();
    ASSERT_EQ(packageSize,
              sizeof(ESSReadout::Parser::PacketHeaderV0) + Sections * (4 + 20));
    auto Res = ESSHeaderParser.validate((char *)socket.GetData().data(),
                                      packageSize, DataType);
    ASSERT_EQ(Res, ESSReadout::Parser::OK);
    Res = CaenParser.parse(ESSHeaderParser.Packet.DataPtr,
                           ESSHeaderParser.Packet.DataLength);
    ASSERT_EQ(Res, Sections);
  }
}

TEST_F(CombinedParserTest, DataGenDefault) {

  SocketMock socket{};
  const std::chrono::microseconds pulseTimeDuration{0};

  std::unique_ptr<FunctionGenerator> readoutTimeGenerator =
      std::make_unique<DistributionGenerator>(
          ReadoutGeneratorBase::DEFAULT_FREQUENCY);

  Caen::ReadoutGenerator gen;
  gen.setReadoutDataSize(sizeof(Caen::DataParser::CaenReadout));
  gen.initialize(std::move(readoutTimeGenerator));

  for (unsigned int Sections = 1; Sections < 372; Sections++) {

    gen.setReadoutPerPacket(Sections);

    socket.Clear();
    gen.generatePackets(&socket, pulseTimeDuration);
    unsigned long packageSize = socket.GetData().size();
    ASSERT_EQ(packageSize,
              sizeof(ESSReadout::Parser::PacketHeaderV1) + Sections * (4 + 20));

    auto Res = ESSHeaderParser.validate((char *)socket.GetData().data(),
                                      packageSize, DataType);
    ASSERT_EQ(Res, ESSReadout::Parser::OK);
    Res = CaenParser.parse(ESSHeaderParser.Packet.DataPtr,
                           ESSHeaderParser.Packet.DataLength);
    ASSERT_EQ(Res, Sections);
  }
}

TEST_F(CombinedParserTest, ParseUDPPacket) {
  auto Res = ESSHeaderParser.validate((char *)&UdpPayload[0], UdpPayload.size(),
                                    DataType);
  ASSERT_EQ(Res, ESSReadout::Parser::OK);
  Res = CaenParser.parse(ESSHeaderParser.Packet.DataPtr,
                         ESSHeaderParser.Packet.DataLength);
  ASSERT_EQ(Res, 2);

  // Just for visual inspection for now
  for (auto const &Data : CaenParser.Result) {
    printf("Fiber %u, FEN %u\n", Data.FiberId, Data.FENId);
    printf("time (%u, %u), SeqNum %u, Group %u, A %u, B %u, C %u, D %u\n",
           Data.TimeHigh, Data.TimeLow, Data.Unused, Data.Group, Data.AmpA,
           Data.AmpB, Data.AmpC, Data.AmpD);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
