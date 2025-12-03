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
using namespace vmm3;
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
  const VMM3Geometry &Geom = Freia->getGeometry();
  ASSERT_EQ(Geom.getBaseCounters().RingErrors, 0);
  ASSERT_EQ(Geom.getBaseCounters().FENErrors, 0);

  Freia->processReadouts();
  ASSERT_EQ(Geom.getVmmCounters().HybridMappingErrors, 1);
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

//===----------------------------------------------------------------------===//
/// \brief Test cases for geometry creation (createGeometry method)
/// These tests verify the FREIA and AMOR instrument geometry instantiation
/// as specified in the selected code block in FreiaInstrument.h (lines 61-69)
//===----------------------------------------------------------------------===//

TEST_F(FreiaInstrumentTest, CreateFreiaGeometry) {
  /// Test that FREIA detector with FREIA geometry creates FreiaGeometry
  /// This covers: DetectorType::FREIA + "FREIA" -> FreiaGeometry
  Settings.ConfigFile = FREIA_FULL;
  Settings.CalibFile = CalibFile;
  
  FreiaInstrument FreiaWithFreia(Counters, Settings, serializer, ESSHeaderParser,
                                 Stats, DetectorType::FREIA);
  const VMM3Geometry &Geom = FreiaWithFreia.getGeometry();
  
  // Verify that the geometry is the expected concrete type
  // returns nullptr if the cast fails
  const FreiaGeometry *GeometryPtr = dynamic_cast<const FreiaGeometry *>(&Geom);
  ASSERT_NE(GeometryPtr, nullptr);
}

TEST_F(FreiaInstrumentTest, CreateAmorGeometry) {
  /// Test that FREIA detector with AMOR geometry creates AmorGeometry
  /// This covers: DetectorType::FREIA + "AMOR" -> AmorGeometry
  Settings.ConfigFile = AMOR_FULL;
  Settings.CalibFile = "";  // Skip calibration to avoid hybrid ID mismatch
  
  FreiaInstrument FreiaWithAmor(Counters, Settings, serializer, ESSHeaderParser,
                                Stats, DetectorType::FREIA);
  const VMM3Geometry &Geom = FreiaWithAmor.getGeometry();
  
  // Verify that the geometry is the expected concrete type
  const AmorGeometry *GeometryPtr = dynamic_cast<const AmorGeometry *>(&Geom);
  ASSERT_NE(GeometryPtr, nullptr);
}

TEST_F(FreiaInstrumentTest, CreateCaseInsensitiveAmorGeometry) {
  /// Test that FREIA detector geometry comparison is case-insensitive
  /// The createGeometry method converts geometry name to uppercase before
  /// comparison, so both "amor" and "AMOR" should create AmorGeometry
  Settings.ConfigFile = AMOR_FULL;
  Settings.CalibFile = "";  // Skip calibration to avoid hybrid ID mismatch
  
  // AMOR_FULL config contains "InstrumentGeometry" : "AMOR"
  // Test that this is properly loaded and compared (case-insensitive)
  FreiaInstrument FreiaWithAmor(Counters, Settings, serializer, ESSHeaderParser,
                                Stats, DetectorType::FREIA);
  const VMM3Geometry &Geom = FreiaWithAmor.getGeometry();
  
  // Verify successful creation and correct concrete type despite case handling
  // returns nullptr if the cast fails
  const AmorGeometry *GeometryPtr = dynamic_cast<const AmorGeometry *>(&Geom);
  ASSERT_NE(GeometryPtr, nullptr);
}

TEST_F(FreiaInstrumentTest, CreateInvalidFREIAGeometry) {
  /// Test that FREIA detector with invalid geometry throws exception
  /// This test uses a valid config that passes loading but has invalid InstrumentGeometry
  
  std::string InvalidFREIAConfig{"deleteme_freia_invalid_geometry.json"};
  std::string InvalidFREIAConfigStr = R"(
{
  "Detector" : "Freia",
  "InstrumentGeometry" : "InvalidGeometry",
  "Version" : 1,
  "Comment" : "Test config with invalid geometry",
  "Date" : "2025-11-11",
  "Info" : "Test config - FREIA requires FREIA or AMOR geometry",
  "WireChOffset" : 16,
  "Config" : [
    { "CassetteNumber":  0, "Ring" :  0, "FEN": 0, "Hybrid" :  0, "Thresholds" : [[0],[0]], "HybridId" : "E5533333222222221111111100000000"}
  ]
}
  )";
  
  saveBuffer(InvalidFREIAConfig, (void *)InvalidFREIAConfigStr.c_str(), 
             InvalidFREIAConfigStr.size());
  
  Settings.ConfigFile = InvalidFREIAConfig;
  Settings.CalibFile = "";
  
  // Expect runtime_error for invalid geometry with FREIA
  try {
    FreiaInstrument InvalidFreia(Counters, Settings, serializer, 
                                 ESSHeaderParser, Stats, 
                                 DetectorType::FREIA);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    std::string error_msg(e.what());
    // Verify exact error message
    EXPECT_EQ(error_msg, "FREIA detector requires InstrumentGeometry 'AMOR' or 'Freia', got: InvalidGeometry");
  }
  
  deleteFile(InvalidFREIAConfig);
}

//===----------------------------------------------------------------------===//
/// \brief Test cases for TBLMB detector geometry creation
/// Tests the TBLMB branch of the createGeometry method (lines 71-78)
//===----------------------------------------------------------------------===//

TEST_F(FreiaInstrumentTest, CreateTBLMBGeometry) {
  /// Test that TBLMB detector with AMOR geometry creates TBLMBGeometry
  /// This covers: DetectorType::TBLMB + "AMOR" -> TBLMBGeometry
  Settings.ConfigFile = TBLMB_FULL;
  Settings.CalibFile = "";  // Skip calibration to avoid hybrid ID mismatch
  
  FreiaInstrument TBLMBWithAmor(Counters, Settings, serializer, ESSHeaderParser,
                                Stats, DetectorType::TBLMB);
  const VMM3Geometry &Geom = TBLMBWithAmor.getGeometry();
  
  // Verify that the geometry is the expected concrete type
  // returns nullptr if the cast fails
  const TBLMBGeometry *GeometryPtr = dynamic_cast<const TBLMBGeometry *>(&Geom);
  ASSERT_NE(GeometryPtr, nullptr);
}

TEST_F(FreiaInstrumentTest, CreateInvalidTBLMBGeometry) {
  /// Test that TBLMB detector with non-AMOR geometry throws exception
  /// This test uses a valid config that passes loading but has invalid InstrumentGeometry
  
  std::string InvalidTBLMBConfig{"deleteme_tblmb_invalid_geometry.json"};
  std::string InvalidTBLMBConfigStr = R"(
{
  "Detector" : "Freia",
  "InstrumentGeometry" : "InvalidGeometry",
  "Version" : 1,
  "Comment" : "Test config with invalid geometry",
  "Date" : "2025-11-11",
  "Info" : "Test config - TBLMB requires AMOR geometry",
  "WireChOffset" : 16,
  "Config" : [
    { "CassetteNumber":  0, "Ring" :  0, "FEN": 0, "Hybrid" :  0, "Thresholds" : [[0],[0]], "HybridId" : "E5533333222222221111111100000000"}
  ]
}
  )";
  
  saveBuffer(InvalidTBLMBConfig, (void *)InvalidTBLMBConfigStr.c_str(), 
             InvalidTBLMBConfigStr.size());
  
  Settings.ConfigFile = InvalidTBLMBConfig;
  Settings.CalibFile = "";
  
  // Expect runtime_error for non-AMOR geometry with TBLMB
  try {
    FreiaInstrument InvalidTBLMB(Counters, Settings, serializer, 
                                 ESSHeaderParser, Stats, 
                                 DetectorType::TBLMB);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    std::string error_msg(e.what());
    // Verify exact error message
    EXPECT_EQ(error_msg, "TBLMB detector requires InstrumentGeometry 'AMOR', got: InvalidGeometry");
  }
  
  deleteFile(InvalidTBLMBConfig);
}

//===----------------------------------------------------------------------===//
/// \brief Test cases for ESTIA detector geometry creation
/// Tests the ESTIA branch of the createGeometry method (lines 80-87)
/// Note: The current implementation has a comparison bug (detectorType != UpperInstGeometry)
/// which should be (UpperInstGeometry != "ESTIA"), but we test what's actually there
//===----------------------------------------------------------------------===//

TEST_F(FreiaInstrumentTest, CreateEstiaGeometry) {
  /// Test that ESTIA detector with ESTIA geometry creates EstiaGeometry
  /// This covers: DetectorType::ESTIA -> EstiaGeometry (if the comparison passes)
  Settings.ConfigFile = ESTIA_FULL;
  Settings.CalibFile = "";  // Skip calibration to avoid hybrid ID mismatch
  
  FreiaInstrument EstiaWithEstia(Counters, Settings, serializer, ESSHeaderParser,
                                 Stats, DetectorType::ESTIA);
  const VMM3Geometry &Geom = EstiaWithEstia.getGeometry();
  
  // Verify that the geometry is the expected concrete type
  // returns nullptr if the cast fails
  const EstiaGeometry *GeometryPtr = dynamic_cast<const EstiaGeometry *>(&Geom);
  ASSERT_NE(GeometryPtr, nullptr);
}

TEST_F(FreiaInstrumentTest, CreateInvalidEstiaGeometry) {
  /// Test that ESTIA detector with non-ESTIA geometry throws exception
  /// This test uses a valid config that passes loading but has invalid InstrumentGeometry
  /// Note: Current implementation has detectorType != UpperInstGeometry comparison
  
  std::string InvalidEstiaConfig{"deleteme_estia_invalid_geometry.json"};
  std::string InvalidEstiaConfigStr = R"(
{
  "Detector" : "Freia",
  "InstrumentGeometry" : "InvalidGeometry",
  "Version" : 1,
  "Comment" : "Test config with invalid geometry",
  "Date" : "2025-11-11",
  "Info" : "Test config - ESTIA requires ESTIA geometry",
  "WireChOffset" : 16,
  "Config" : [
    { "CassetteNumber":  0, "Ring" :  0, "FEN": 0, "Hybrid" :  0, "Thresholds" : [[0],[0]], "HybridId" : "E5533333222222221111111100000000"}
  ]
}
  )";
  
  saveBuffer(InvalidEstiaConfig, (void *)InvalidEstiaConfigStr.c_str(), 
             InvalidEstiaConfigStr.size());
  
  Settings.ConfigFile = InvalidEstiaConfig;
  Settings.CalibFile = "";
  
  // Expect runtime_error for non-ESTIA geometry with ESTIA detector
  try {
    FreiaInstrument InvalidEstia(Counters, Settings, serializer, 
                                 ESSHeaderParser, Stats, 
                                 DetectorType::ESTIA);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    std::string error_msg(e.what());
    // Verify exact error message
    EXPECT_EQ(error_msg, "ESTIA detector requires InstrumentGeometry 'ESTIA', got: InvalidGeometry");
  }
  
  deleteFile(InvalidEstiaConfig);
}

TEST_F(FreiaInstrumentTest, CreateUnsupportedDetectorType) {
  /// Test that unsupported DetectorType throws exception
  /// This test uses a valid config but BIFROST detector is not supported
  /// This covers the final error case: throw std::runtime_error("Unsupported DetectorType...")
  
  std::string ValidConfig{"deleteme_unsupported_detector.json"};
  std::string ValidConfigStr = R"(
{
  "Detector" : "Freia",
  "InstrumentGeometry" : "Freia",
  "Version" : 1,
  "Comment" : "Valid config but unsupported DetectorType",
  "Date" : "2025-11-11",
  "Info" : "Test config - valid config but unsupported DetectorType",
  "WireChOffset" : 16,
  "Config" : [
    { "CassetteNumber":  0, "Ring" :  0, "FEN": 0, "Hybrid" :  0, "Thresholds" : [[0],[0]], "HybridId" : "E5533333222222221111111100000000"}
  ]
}
  )";
  
  saveBuffer(ValidConfig, (void *)ValidConfigStr.c_str(), 
             ValidConfigStr.size());
  
  Settings.ConfigFile = ValidConfig;
  Settings.CalibFile = "";
  
  // Expect runtime_error for unsupported detector type
  try {
    FreiaInstrument UnsupportedDetector(Counters, Settings, serializer, 
                                        ESSHeaderParser, Stats, 
                                        DetectorType::BIFROST);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    std::string error_msg(e.what());
    // Verify exact error message
    EXPECT_EQ(error_msg, "Unsupported DetectorType: BIFROST");
  }
  
  deleteFile(ValidConfig);
}

int main(int argc, char **argv) {
  saveBuffer(CalibFile, (void *)CalibStr.c_str(), CalibStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(CalibFile);
  return RetVal;
}
