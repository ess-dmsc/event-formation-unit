// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/kafka/EV44Serializer.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/HeaderFactory.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <estia/EstiaInstrument.h>

using namespace Estia;
using namespace ESSReadout;
// clang-format off

std::string CalibFile{"deleteme_estia_instr_calib.json"};
std::string CalibStr = R"(
  {
  "Detector" : "Estia",
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

class EstiaInstrumentTest : public TestBase {
public:
protected:
  struct Counters counters;
  BaseSettings Settings;
  EV44Serializer *serializer;
  EstiaInstrument *estia;
  std::unique_ptr<TestHeaderFactory> headerFactory;
  Event TestEvent;           // used for testing generateEvents()
  std::vector<Event> Events; // used for testing generateEvents()

  void SetUp() override {
    Settings.ConfigFile = ESTIA_FULL;
    serializer = new EV44Serializer(115000, "estia");
    counters = {};

    headerFactory = std::make_unique<TestHeaderFactory>();
    estia = new EstiaInstrument(counters, Settings, serializer);
    estia->setSerializer(serializer);
    estia->ESSReadoutParser.Packet.HeaderPtr =
        headerFactory->createHeader(ESSReadout::Parser::V1);
  }
  void TearDown() override {}

  void makeHeader(ESSReadout::Parser::PacketDataV0 &Packet,
                  std::vector<uint8_t> &testdata) {
    Packet.HeaderPtr = headerFactory->createHeader(ESSReadout::Parser::V1);
    Packet.DataPtr = (char *)&testdata[0];
    Packet.DataLength = testdata.size();
    Packet.Time.setReference(ESSTime(0, 0));
    Packet.Time.setPrevReference(ESSTime(0, 0));
  }
};

// Test cases below
TEST_F(EstiaInstrumentTest, Constructor) {
  Settings.CalibFile = CalibFile;
  EstiaInstrument Estia(counters, Settings, serializer);
  counters.VMMStats = estia->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
}

/// THIS IS NOT A TEST, just ensure we also try dumping to hdf5
TEST_F(EstiaInstrumentTest, DumpTofile) {
  Settings.DumpFilePrefix = "deleteme_";
  EstiaInstrument EstiaDump(counters, Settings, serializer);
  EstiaDump.setSerializer(serializer);

  makeHeader(EstiaDump.ESSReadoutParser.Packet, GoodEvent);
  auto Res = EstiaDump.VMMParser.parse(EstiaDump.ESSReadoutParser.Packet);
  EstiaDump.processReadouts();

  counters.VMMStats = EstiaDump.VMMParser.Stats;
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(counters.VMMStats.Readouts, 2);
}

TEST_F(EstiaInstrumentTest, MappingError) {
  makeHeader(estia->ESSReadoutParser.Packet, MappingError);
  auto Res = estia->VMMParser.parse(estia->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 3);
  counters.VMMStats = estia->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.HybridMappingErrors, 0);
  ASSERT_EQ(counters.RingMappingErrors, 0);
  ASSERT_EQ(counters.FENMappingErrors, 0);

  estia->processReadouts();
  ASSERT_EQ(counters.HybridMappingErrors, 1);
  ASSERT_EQ(counters.RingMappingErrors, 1);
  ASSERT_EQ(counters.FENMappingErrors, 1);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 0);
  ASSERT_EQ(counters.VMMStats.ErrorFiber, 0);
}

TEST_F(EstiaInstrumentTest, MaxRingMaxFENErrors) {
  makeHeader(estia->ESSReadoutParser.Packet, MaxRingMaxFENErrors);
  auto Res = estia->VMMParser.parse(estia->ESSReadoutParser.Packet);
  ASSERT_EQ(Res, 0);
  counters.VMMStats = estia->VMMParser.Stats;
  ASSERT_EQ(counters.VMMStats.ErrorFiber, 1);
  ASSERT_EQ(counters.VMMStats.ErrorFEN, 1);
}

TEST_F(EstiaInstrumentTest, InvalidWireChannels) {
  makeHeader(estia->ESSReadoutParser.Packet, InvalidChannel);
  auto Res = estia->VMMParser.parse(estia->ESSReadoutParser.Packet);
  counters.VMMStats = estia->VMMParser.Stats;

  ASSERT_EQ(Res, 3);
  ASSERT_EQ(counters.VMMStats.ErrorChannel, 1);

  // Hack last readout have valid channel
  estia->VMMParser.Result[2].Channel = 65;
  estia->processReadouts();

  ASSERT_EQ(counters.InvalidXCoord, 1);
  ASSERT_EQ(counters.InvalidYCoord, 2);
}

TEST_F(EstiaInstrumentTest, WireGap) {
  TestEvent.ClusterA.insert({0, 1, 100, 0});
  TestEvent.ClusterB.insert({0, 1, 100, 1});
  TestEvent.ClusterB.insert({0, 3, 100, 1});
  Events.push_back(TestEvent);

  estia->generateEvents(Events);
  ASSERT_EQ(counters.EventsInvalidWireGap, 1);
}

TEST_F(EstiaInstrumentTest, StripGap) {
  TestEvent.ClusterA.insert({0, 1, 100, 0});
  TestEvent.ClusterA.insert({0, 3, 100, 0});
  TestEvent.ClusterB.insert({0, 1, 100, 1});
  Events.push_back(TestEvent);

  estia->generateEvents(Events);
  ASSERT_EQ(counters.EventsInvalidStripGap, 1);
}

TEST_F(EstiaInstrumentTest, ClusterWireOnly) {
  TestEvent.ClusterB.insert({0, 1, 0, 0});
  Events.push_back(TestEvent);

  estia->generateEvents(Events);
  ASSERT_EQ(counters.EventsMatchedWireOnly, 1);
}

TEST_F(EstiaInstrumentTest, ClusterStripOnly) {
  TestEvent.ClusterA.insert({0, 1, 0, 0});
  Events.push_back(TestEvent);

  estia->generateEvents(Events);
  ASSERT_EQ(counters.EventsMatchedStripOnly, 1);
}

TEST_F(EstiaInstrumentTest, PixelError) {
  //                         t  c  w    p
  TestEvent.ClusterA.insert({0, 2000, 100, 0});
  TestEvent.ClusterB.insert({0, 1, 100, 1});
  Events.push_back(TestEvent);
  estia->generateEvents(Events);
  ASSERT_EQ(counters.PixelErrors, 1);
}

TEST_F(EstiaInstrumentTest, EventTOFError) {
  auto &Packet = estia->ESSReadoutParser.Packet;
  makeHeader(Packet, GoodEvent);

  Packet.Time.setReference(ESSTime(200, 0));
  auto Res = estia->VMMParser.parse(Packet);
  counters.VMMStats = estia->VMMParser.Stats;

  estia->processReadouts();
  for (auto &builder : estia->builders) {
    builder.flush(true);
    estia->generateEvents(builder.Events);
  }
  ASSERT_EQ(Res, 2);
  ASSERT_EQ(counters.VMMStats.Readouts, 2);
  ASSERT_EQ(counters.TimeErrors, 1);
}

TEST_F(EstiaInstrumentTest, GoodEvent) {
  //                         t  c  w    p
  TestEvent.ClusterA.insert({0, 3, 100, 0});
  TestEvent.ClusterB.insert({0, 1, 100, 1});
  Events.push_back(TestEvent);
  estia->generateEvents(Events);
  ASSERT_EQ(counters.Events, 1);
}

TEST_F(EstiaInstrumentTest, EventTOFTooLarge) {
  TestEvent.ClusterA.insert({3000000000, 3, 100, 0});
  TestEvent.ClusterB.insert({3000000000, 1, 100, 1});
  Events.push_back(TestEvent);
  estia->generateEvents(Events);
  ASSERT_EQ(counters.Events, 0);
  ASSERT_EQ(counters.MaxTOFErrors, 1);
}

TEST_F(EstiaInstrumentTest, NoEvents) {
  Events.push_back(TestEvent);
  estia->generateEvents(Events);
  ASSERT_EQ(counters.Events, 0);
}

int main(int argc, char **argv) {
  saveBuffer(CalibFile, (void *)CalibStr.c_str(), CalibStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(CalibFile);
  return RetVal;
}
