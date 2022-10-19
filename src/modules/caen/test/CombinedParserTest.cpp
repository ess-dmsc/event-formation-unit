// Copyright (C) 2019 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests
///
//===----------------------------------------------------------------------===//

#include <common/readout/ess/Parser.h>
#include <common/testutils/TestBase.h>
#include <caen/Counters.h>
#include <caen/generators/LokiReadoutGenerator.h>
#include <caen/readout/DataParser.h>

const uint32_t FirstSeqNum{0};

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
  // From Counters.h
  struct Counters Counters;

  const int DataType{0x30};
  ESSReadout::Parser CommonReadout;
  DataParser CaenParser{Counters};

  void SetUp() override {
    Counters = {};
    CommonReadout.setMaxPulseTimeDiff(4000000000);
  }
  void TearDown() override {}
};

// Cycle through all section values with equal number of readouts
TEST_F(CombinedParserTest, DataGen) {
  const uint16_t BufferSize{8972};
  uint8_t Buffer[BufferSize];
  uint32_t SeqNum{FirstSeqNum};
  ReadoutGeneratorBase::GeneratorSettings Settings;

  for (unsigned int Sections = 1; Sections < 372; Sections++) {
    Settings.NumReadouts = Sections;
    Caen::LokiReadoutGenerator gen(Buffer, BufferSize, SeqNum, Settings);
    gen.setReadoutDataSize(sizeof(Caen::DataParser::CaenReadout));
    Settings.Type = ESSReadout::Parser::DetectorType::Loki4Amp;

    uint16_t DataSize = gen.makePacket();
    ASSERT_EQ(DataSize,
              sizeof(ESSReadout::Parser::PacketHeaderV0) + Sections * (4 + 20));

    auto Res = CommonReadout.validate((char *)&Buffer[0], DataSize, DataType);
    ASSERT_EQ(Res, ESSReadout::Parser::OK);
    Res = CaenParser.parse(CommonReadout.Packet.DataPtr,
                           CommonReadout.Packet.DataLength);
    ASSERT_EQ(Res, Sections);
  }
}

TEST_F(CombinedParserTest, ParseUDPPacket) {
  auto Res = CommonReadout.validate((char *)&UdpPayload[0], UdpPayload.size(),
                                    DataType);
  ASSERT_EQ(Res, ESSReadout::Parser::OK);
  Res = CaenParser.parse(CommonReadout.Packet.DataPtr,
                         CommonReadout.Packet.DataLength);
  ASSERT_EQ(Res, 2);

  // Just for visual inspection for now
  for (auto &Data : CaenParser.Result) {
    printf("Ring %u, FEN %u\n", Data.RingId, Data.FENId);
    printf("time (%u, %u), SeqNum %u, Tube %u, A %u, B %u, C %u, D %u\n",
           Data.TimeHigh, Data.TimeLow, Data.DataSeqNum, Data.TubeId, Data.AmpA,
           Data.AmpB, Data.AmpC, Data.AmpD);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
