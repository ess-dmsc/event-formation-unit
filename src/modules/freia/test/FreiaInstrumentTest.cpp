// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for FreiaInstrument
//===----------------------------------------------------------------------===//

#include <common/Statistics.h>
#include <common/kafka/EV44Serializer.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/HeaderFactory.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <common/types/DetectorType.h>

#include <freia/Counters.h>
#include <freia/FreiaBase.h>
#include <freia/FreiaInstrument.h>
#include <memory>

using namespace Freia;
using namespace ESSReadout;
// clang-format off

std::string CalibFile{"deleteme_freia_instr_calib.json"};
std::string CalibStr = R"(
  {
  "Detector" : "Freia",
  "Version"  : 1,
  "Date"     : "some day",
  "Info"     : "Some info",
  "Hybrids" : 32,
  "Comment" : "Artificial calibration file - unity (slope 1, offset 0) ",

  "Calibrations" : [
    { "VMMHybridCalibration" : {
        "HybridId" : "E5533333222222221111111100000000",
        "CalibrationDate" : "20210222-124533",

        "vmm0" : {
          "Settings" : "all ain and no pain",
          "adc_offset" : [50.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
          "adc_slope"  : [5.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
          "tdc_offset" : [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
          "tdc_slope"  : [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
          "tdc_ofs_corr" : [0.0, 1.0]
        },
        "vmm1" : {
          "Settings" : "all ain and no pain",
          "adc_offset" : [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
          "adc_slope"  : [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
          "tdc_offset" : [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
          "tdc_slope"  : [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
          "tdc_ofs_corr" : [0.0, 1.0]
        }
      }
    }
    ]
  }
)";

//

std::vector<uint8_t> MaxRingMaxFENErrors{
    // First readout
    0x18, 0x00, 0x14, 0x00, // Data Header - Ring 24 is greater than max ring number
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x00, 0x10, // GEO 0, TDC 0, VMM 0, CH 16

    // Second readout
    0x02, 0x18, 0x14, 0x00, // Data Header - FEN 24 is greater than max FEN number
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x11, 0x00, 0x00, 0x00, // Time LO 17 ticka
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x01, 0x10, // GEO 0, TDC 0, VMM 1, CH 16
};

std::vector<uint8_t> MappingError{
    // First readout - RingMappingErrors
    0x16, 0x00, 0x14, 0x00, // Data Header - Ring 22 is reserved for monitor
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x04, 0x10, // GEO 0, TDC 0, VMM 2, CH 16

    // Second readout - FENMappingError
    0x00, 0x0F, 0x14, 0x00, // Data Header - FEN 15 is greater config
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x01, 0x00, 0x00, 0x00, // Time LO 17 ticka
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x04, 0x10, // GEO 0, TDC 0, VMM 2, CH 16

    // Third readout - HybridMappingError
    0x01, 0x02, 0x14, 0x00, // Data Header - Ring1, FEN 2 is bad config
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x01, 0x00, 0x00, 0x00, // Time LO 17 ticka
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x04, 0x10, // GEO 0, TDC 0, VMM 2, CH 16
};

std::vector<uint8_t> GoodEvent{
    // First readout - plane Y - Wires
    0x04, 0x00, 0x14, 0x00, // Data Header - Ring 4, FEN 0
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x00, 0x10, // GEO 0, TDC 0, VMM 0, CH 16

    // Second readout - plane X - Strips
    0x05, 0x00, 0x14, 0x00, // Data Header, Ring 5, FEN 0
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x11, 0x00, 0x00, 0x00, // Time LO 17 ticka
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x01, 0x10, // GEO 0, TDC 0, VMM 1, CH 16
};

std::vector<uint8_t> InvalidChannel{
    // First readout - plane Y - Wires - Fail in processing with invalid channel
    0x04, 0x00, 0x14, 0x00, // Data Header - Ring 4, FEN 0
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x00, 0x0F, // GEO 0, TDC 0, VMM 0, CH 15

    // Second readout - plane Y - Wires - Fail in processing with invalid channel
    0x05, 0x00, 0x14, 0x00, // Data Header, Ring 5, FEN 0
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x11, 0x00, 0x00, 0x00, // Time LO 17 ticka
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x00, 0x30, // GEO 0, TDC 0, VMM 1, CH 48

    // Second readout - plane X - Strips - Fail in parsing with channel error
    0x05, 0x00, 0x14, 0x00, // Data Header, Ring 5, FEN 0
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x11, 0x00, 0x00, 0x00, // Time LO 17 ticka
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x01, 0x41, // GEO 0, TDC 0, VMM 1, CH 65

    // Second readout - plane X - Strips - Let it pass to overwide the channel
    0x05, 0x00, 0x14, 0x00, // Data Header, Ring 5, FEN 0
    0x00, 0x00, 0x00, 0x00, // Time HI 0 s
    0x11, 0x00, 0x00, 0x00, // Time LO 17 ticka
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x01, 0x00, // GEO 0, TDC 0, VMM 1, CH 0
};
// clang-format on

class FreiaInstrumentTest : public TestBase {
public:
protected:
  struct Counters Counters;
  BaseSettings Settings;
  Statistics Stats;
  ESSReadout::Parser ESSHeaderParser{Stats};
  EV44Serializer serializer{115000, "freia"};
  std::unique_ptr<FreiaInstrument> Freia;
  std::unique_ptr<TestHeaderFactory> HeaderFactory;
  Event TestEvent;           // used for testing generateEvents()
  std::vector<Event> Events; // used for testing generateEvents()

  void SetUp() override {
    Settings.ConfigFile = FREIA_FULL;
    Counters = {};

    HeaderFactory = std::make_unique<TestHeaderFactory>();
    Freia = std::make_unique<FreiaInstrument>(Counters, Settings, serializer,
                                              ESSHeaderParser, Stats, 
                                              DetectorType::FREIA);
    ESSHeaderParser.Packet.HeaderPtr =
        HeaderFactory->createHeader(ESSReadout::Parser::V1);
  }
  void TearDown() override {}

  void makeHeader(ESSReadout::Parser::PacketDataV0 &Packet,
                  std::vector<uint8_t> &testdata) {
    Packet.HeaderPtr = HeaderFactory->createHeader(ESSReadout::Parser::V1);
    Packet.DataPtr = (char *)&testdata[0];
    Packet.DataLength = testdata.size();
    Packet.Time.setReference(ESSTime(0, 0));
    Packet.Time.setPrevReference(ESSTime(0, 0));
  }
};

// Test cases below
TEST_F(FreiaInstrumentTest, Constructor) {
  Settings.CalibFile = CalibFile;
  FreiaInstrument Freia(Counters, Settings, serializer, ESSHeaderParser, Stats,
                        DetectorType::FREIA);
  Counters.VMMStats = Freia.VMMParser.Stats;
  ASSERT_EQ(Counters.VMMStats.ErrorFiber, 0);
}

TEST_F(FreiaInstrumentTest, MappingError) {
  makeHeader(ESSHeaderParser.Packet, MappingError);
  auto Res = Freia->VMMParser.parse(ESSHeaderParser.Packet);
  ASSERT_EQ(Res, 3);
  Counters.VMMStats = Freia->VMMParser.Stats;
  ASSERT_EQ(Counters.VMMStats.ErrorFiber, 0);
  ASSERT_EQ(Counters.VMMStats.ErrorFEN, 0);
  const GeometryBase &Geom = Freia->getGeometry();
  ASSERT_EQ(Geom.getBaseCounters().RingErrors, 0);
  ASSERT_EQ(Geom.getBaseCounters().FENErrors, 0);

  Freia->processReadouts();
  ASSERT_EQ(Geom.getVmmGeometryCounters().HybridMappingErrors, 1);
  ASSERT_EQ(Geom.getBaseCounters().RingErrors, 1);
  ASSERT_EQ(Geom.getBaseCounters().FENErrors, 1);
  ASSERT_EQ(Counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(Counters.VMMStats.ErrorFiber, 0);
}

TEST_F(FreiaInstrumentTest, MaxRingMaxFENErrors) {
  makeHeader(ESSHeaderParser.Packet, MaxRingMaxFENErrors);
  auto Res = Freia->VMMParser.parse(ESSHeaderParser.Packet);
  ASSERT_EQ(Res, 0);
  Counters.VMMStats = Freia->VMMParser.Stats;
  ASSERT_EQ(Counters.VMMStats.ErrorFiber, 1);
  ASSERT_EQ(Counters.VMMStats.ErrorFEN, 1);
}

TEST_F(FreiaInstrumentTest, InvalidWireChannels) {
  makeHeader(ESSHeaderParser.Packet, InvalidChannel);
  auto Res = Freia->VMMParser.parse(ESSHeaderParser.Packet);
  Counters.VMMStats = Freia->VMMParser.Stats;

  ASSERT_EQ(Res, 3);
  ASSERT_EQ(Counters.VMMStats.ErrorChannel, 1);

  // Hack last readout have valid channel
  Freia->VMMParser.Result[2].Channel = 65;
  Freia->processReadouts();

  auto GeomStats = Freia->getVmmGeometryStats();
  ASSERT_EQ(GeomStats.InvalidXCoord, 1);
  ASSERT_EQ(GeomStats.InvalidYCoord, 2);
}

TEST_F(FreiaInstrumentTest, WireGap) {
  TestEvent.ClusterA.insert({0, 1, 100, 0});
  TestEvent.ClusterB.insert({0, 1, 100, 1});
  TestEvent.ClusterB.insert({0, 3, 100, 1});
  Events.push_back(TestEvent);

  Freia->generateEvents(Events);
  ASSERT_EQ(Counters.EventsInvalidWireGap, 1);
}

TEST_F(FreiaInstrumentTest, StripGap) {
  TestEvent.ClusterA.insert({0, 1, 100, 0});
  TestEvent.ClusterA.insert({0, 3, 100, 0});
  TestEvent.ClusterB.insert({0, 1, 100, 1});
  Events.push_back(TestEvent);

  Freia->generateEvents(Events);
  ASSERT_EQ(Counters.EventsInvalidStripGap, 1);
}

TEST_F(FreiaInstrumentTest, ClusterWireOnly) {
  TestEvent.ClusterB.insert({0, 1, 0, 0});
  Events.push_back(TestEvent);

  Freia->generateEvents(Events);
  ASSERT_EQ(Counters.EventsMatchedWireOnly, 1);
}

TEST_F(FreiaInstrumentTest, ClusterStripOnly) {
  TestEvent.ClusterA.insert({0, 1, 0, 0});
  Events.push_back(TestEvent);

  Freia->generateEvents(Events);
  ASSERT_EQ(Counters.EventsMatchedStripOnly, 1);
}

TEST_F(FreiaInstrumentTest, PixelError) {
  //                         t  c  w    p
  TestEvent.ClusterA.insert({0, 2000, 100, 0});
  TestEvent.ClusterB.insert({0, 1, 100, 1});
  Events.push_back(TestEvent);
  Freia->generateEvents(Events);
  auto const &Geom = Freia->getGeometry();
  ASSERT_EQ(Geom.getBaseCounters().PixelErrors, 1);
}

TEST_F(FreiaInstrumentTest, EventTOFError) {
  auto &Packet = ESSHeaderParser.Packet;
  makeHeader(Packet, GoodEvent);

  Packet.Time.setReference(ESSTime(200, 0));
  auto Res = Freia->VMMParser.parse(Packet);
  Counters.VMMStats = Freia->VMMParser.Stats;

  Freia->processReadouts();
  for (auto &builder : Freia->builders) {
    builder.flush(true);
    Freia->generateEvents(builder.Events);
  }
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(Counters.VMMStats.Readouts, 2);
  ASSERT_EQ(Counters.TimeErrors, 1);
}

TEST_F(FreiaInstrumentTest, GoodEvent) {
  //                         t  c  w    p
  TestEvent.ClusterA.insert({0, 3, 100, 0});
  TestEvent.ClusterB.insert({0, 1, 100, 1});
  Events.push_back(TestEvent);
  Freia->generateEvents(Events);
  ASSERT_EQ(Counters.Events, 1);
}

TEST_F(FreiaInstrumentTest, EventTOFTooLarge) {
  TestEvent.ClusterA.insert({3000000000, 3, 100, 0});
  TestEvent.ClusterB.insert({3000000000, 1, 100, 1});
  Events.push_back(TestEvent);
  Freia->generateEvents(Events);
  ASSERT_EQ(Counters.Events, 0);
  ASSERT_EQ(Counters.MaxTOFErrors, 1);
}

TEST_F(FreiaInstrumentTest, NoEvents) {
  Events.push_back(TestEvent);
  Freia->generateEvents(Events);
  ASSERT_EQ(Counters.Events, 0);
}

int main(int argc, char **argv) {
  saveBuffer(CalibFile, (void *)CalibStr.c_str(), CalibStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(CalibFile);
  return RetVal;
}
