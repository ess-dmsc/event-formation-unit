// Copyright (C) 2021 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Unit test for the CbmInstrument class.
//===----------------------------------------------------------------------===//

#include <common/detector/BaseSettings.h>
#include <common/geometry/DetectorGeometry.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/EV44SerializerMock.h>
#include <common/testutils/HeaderFactory.h>
#include <common/testutils/TestBase.h>
#include <memory>
#include <modules/cbm/CbmInstrument.h>

using namespace cbm;
using namespace ESSReadout;
using namespace testing;

using std::filesystem::path;
using ESSParser = ESSReadout::Parser;

// clang-format off

/// \brief Monitor readout with valid Event0D readouts
/// Tests schema routing: EV44 (FEN 0, Ch 0) and DA00 (FEN 3, Ch 3)
std::vector<uint8_t> ValidEvent0DReadouts {
  // EV44 schema readout
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // DA00 schema readout
  0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA2, 0x86, 0x01, 0x00,  // Time LO 100002 tick
  0x01, 0x03, 0x01, 0x00,  // Type 1, Ch 3, ADC 1
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

/// \brief Monitor readout with valid Event2D readout
/// Tests EV44 schema routing (only schema available for EVENT_2D)
std::vector<uint8_t> ValidEvent2DReadouts {
  0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
  0x02, 0x00, 0x01, 0x00,  // Type 2, Ch 0, ADC 1
  0x00, 0x01, 0x01, 0x01   // XPos 256, YPos 257
};

/// \brief Monitor readout with valid Event2D readouts with time values
std::vector<uint8_t> InvalidPosEvent2DReadouts {
  0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
  0x02, 0x00, 0x01, 0x00,  // Type 2, Ch 2, ADC 1
  0x00, 0x0F, 0x01, 0x01,  // XPos 3840, YPos 257

  0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA2, 0x86, 0x01, 0x00,  // Time LO 100002 tick
  0x02, 0x01, 0x01, 0x00,  // Type 2, Ch 1, ADC 1
  0xAA, 0x01, 0xBB, 0x0F,  // XPos 426, YPos 4027

  0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA3, 0x86, 0x01, 0x00,  // Time LO 100003 tick
  0x02, 0x02, 0x01, 0x00,  // Type 2, Ch 2, ADC 1
  0x01, 0x02, 0x01, 0x02   // XPos 513, YPos 513
};

/// \brief Monitor readout with valid IBM readouts
/// Tests schema routing: DA00 (FEN 1, Ch 1) and EV44 (FEN 3, Ch 4)
std::vector<uint8_t> ValidIBMReadouts {
  // DA00 schema readout
  0x16, 0x01, 0x14, 0x00,  // Fiber 22, FEN 1, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
  0x03, 0x01, 0x00, 0x01,  // Type 3, Ch 1, ADC 1
  0x0A, 0x00, 0x00, 0x00,  // NPOS 10

  // EV44 schema readout
  0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA2, 0x86, 0x01, 0x00,  // Time LO 100002 tick
  0x03, 0x04, 0x01, 0x00,  // Type 3, Ch 4, ADC 1
  0xE8, 0x03, 0x00, 0x00   // NPOS 1000
};

/// \brief Monitor readout with invalid Ring
std::vector<uint8_t> RingNotInCfgReadout {
  0x12, 0x00, 0x14, 0x00,  // Fiber 18, FEN 0, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0
};

/// \brief Monitor readout with invalid Ring for EVENT_0D type
/// Uses valid Fiber IDs but Ring doesn't match configured MonitorRing (11)
std::vector<uint8_t> InvalidRingEvent0DReadouts {
  // Ring = 10 (Fiber 20), doesn't match MonitorRing (11)
  0x14, 0x00, 0x14, 0x00,  // Fiber 20, FEN 0, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Ring = 9 (Fiber 18), doesn't match MonitorRing (11)
  0x12, 0x00, 0x14, 0x00,  // Fiber 18, FEN 0, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA2, 0x86, 0x01, 0x00,  // Time LO 100002 tick
  0x01, 0x01, 0x01, 0x00,  // Type 1, Ch 1, ADC 1
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

/// \brief Monitor readout with invalid Ring for IBM type
/// Uses valid Fiber IDs but Ring doesn't match configured MonitorRing (11)
std::vector<uint8_t> InvalidRingIBMReadouts {
  // Ring = 10 (Fiber 20), doesn't match MonitorRing (11)
  0x14, 0x01, 0x14, 0x00,  // Fiber 20, FEN 1, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
  0x03, 0x01, 0x01, 0x00,  // Type 3, Ch 1, ADC 1
  0x0A, 0x00, 0x00, 0x00,  // NPOS 10

  // Ring = 9 (Fiber 18), doesn't match MonitorRing (11)
  0x12, 0x02, 0x14, 0x00,  // Fiber 18, FEN 2, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA2, 0x86, 0x01, 0x00,  // Time LO 100002 tick
  0x03, 0x01, 0x01, 0x00,  // Type 3, Ch 1, ADC 1
  0xFF, 0xFF, 0xFF, 0x00   // NADC (3 bytes), MCA Sum (0)
};

/// \brief Mixed readouts with validation errors across all types
/// Tests that geometry validation errors are accumulated correctly
std::vector<uint8_t> ValidationErrorsAcrossTypes {
  // EVENT_0D with wrong ring (Ring 10 != MonitorRing 11)
  0x14, 0x00, 0x14, 0x00,  // Fiber 20, FEN 0, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // EVENT_2D with invalid topology (FEN 3, Ch 5 not in config)
  0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA2, 0x86, 0x01, 0x00,  // Time LO 100002 tick
  0x02, 0x05, 0x01, 0x00,  // Type 2, Ch 5, ADC 1
  0x00, 0x01, 0x01, 0x01,  // XPos 256, YPos 257

  // IBM with wrong ring (Ring 9 != MonitorRing 11)
  0x12, 0x01, 0x14, 0x00,  // Fiber 18, FEN 1, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA3, 0x86, 0x01, 0x00,  // Time LO 100003 tick
  0x03, 0x01, 0x01, 0x00,  // Type 3, Ch 1, ADC 1
  0x0A, 0x00, 0x00, 0x00   // NPOS 10
};

/// \brief Monitor readout with invalid FEN, Channel, and Type
/// Tests that topology errors are caught by geometry validation, and
/// invalid types are caught by parser before instrument processing.
std::vector<uint8_t> FenAndChannelNotInCfgReadout {
  // Invalid FEN - not in configuration
  0x16, 0x04, 0x14, 0x00,  // Fiber 22, FEN 4, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x10, 0x00, 0x00, 0x00,  // Time LO 16 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,   // XPos 0, YPos 0

  // Invalid Channel - not in configuration
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x0E, 0x01, 0x00,  // Type 1, Ch 14, ADC 1
  0x00, 0x00, 0x00, 0x00,   // XPos 0, YPos 0

  // Valid Data
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x12, 0x00, 0x00, 0x00,  // Time LO 18 ticks
  0x01, 0x01, 0x01, 0x00,  // Type 1, Ch 1, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Invalid Type - dropped by parser (Type 4 is not valid)
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x13, 0x00, 0x00, 0x00,  // Time LO 19 ticks
  0x04, 0x00, 0x01, 0x00,  // Type 4 (invalid), Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

/// \brief Monitor readout with TOF value higher then MaxTof limit
std::vector<uint8_t> TofToHighReadout {
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x02, 0x00, 0x00, 0x00,  // Time HI 2 s
  0x11, 0xB2, 0x5F, 0x02,   // Time LO 37736785 tick, ~0.4 seconds
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

/// \brief IBM readouts for normalization tests
/// Tests NPOSCount counter accumulation with multiple readouts
/// Readout 1: NADC = 0x001000 (4096), MCASum = 2 -> normalized = 2048
/// Readout 2: NADC = 0x002000 (8192), MCASum = 4 -> normalized = 2048
/// Readout 3: NADC = 0x001000 (4096), MCASum = 0 -> raw value 4096 (division skipped)
std::vector<uint8_t> IBMNormalizeReadouts {
  // First IBM readout: NADC = 4096, MCASum = 2
  0x16, 0x02, 0x14, 0x00,  // Fiber 22, FEN 2, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
  0x03, 0x01, 0x01, 0x00,  // Type 3, Ch 1, ADC 1
  0x00, 0x10, 0x00, 0x02,  // NADC = 0x001000 (4096), MCA Sum = 2

  // Second IBM readout: NADC = 8192, MCASum = 4
  0x16, 0x02, 0x14, 0x00,  // Fiber 22, FEN 2, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA2, 0x86, 0x01, 0x00,  // Time LO 100002 tick
  0x03, 0x01, 0x01, 0x00,  // Type 3, Ch 1, ADC 1
  0x00, 0x20, 0x00, 0x04,  // NADC = 0x002000 (8192), MCA Sum = 4

  // Third IBM readout: NADC = 4096, MCASum = 0 (edge case - division skipped)
  0x16, 0x02, 0x14, 0x00,  // Fiber 22, FEN 2, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA3, 0x86, 0x01, 0x00,  // Time LO 100003 tick
  0x03, 0x01, 0x01, 0x00,  // Type 3, Ch 1, ADC 1
  0x00, 0x10, 0x00, 0x00   // NADC = 0x001000 (4096), MCA Sum = 0
};

/// \brief Monitor readout for TOF fallback tests
/// Single readout at Time = 2s + 90000 ticks
/// By adjusting reference times, can test both PrevTof fallback and negative PrevTof
std::vector<uint8_t> TimingErrorReadout {
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x02, 0x00, 0x00, 0x00,  // Time HI 2 s
  0x09, 0x5F, 0x01, 0x00,  // Time LO 90000 tick
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

auto TestConfig = R"(
{
  "Detector" : "CBM",
  "TypeSubType" : 16,
  "NumberOfMonitors" : 11,
  "MaxPulseTimeDiffNS" : 1000000000,
  "MaxPulseTimeNS" : 357142855,
  "MaxFENId" : 4,
  "MaxFEN" : 16,
  "MonitorRing" : 11,
  "MaxRing" : 11,
  "NormalizeIBMReadouts": true,

  "Topology" : [
    { "FEN": 0, "Channel": 0, "Type": "EVENT_0D", "Source" : "cbm1", "Schema": "ev44", "PixelOffset": 0 },
    { "FEN": 0, "Channel": 1, "Type": "EVENT_0D", "Source" : "cbm2", "Schema": "ev44", "PixelOffset": 1 },
    { "FEN": 1, "Channel": 0, "Type": "EVENT_0D", "Source" : "cbm3", "Schema": "ev44", "PixelOffset": 3 },
    { "FEN": 1, "Channel": 1, "Type": "IBM", "Source" : "cbm4", "Schema": "da00", "MaxTofBin": 10000, "BinCount": 100 },
    { "FEN": 1, "Channel": 2, "Type": "IBM", "Source" : "cbm5", "Schema": "da00", "MaxTofBin": 10000, "BinCount": 100 },
    { "FEN": 2, "Channel": 1, "Type": "IBM", "Source" : "cbm6", "Schema": "da00", "MaxTofBin": 10000, "BinCount": 100 },
    { "FEN": 3, "Channel": 0, "Type": "EVENT_2D", "Source" : "cbm7", "Schema": "ev44", "Width": 512, "Height": 512 },
    { "FEN": 3, "Channel": 1, "Type": "EVENT_2D", "Source" : "cbm8", "Schema": "ev44", "Width": 512, "Height": 512 },
    { "FEN": 3, "Channel": 2, "Type": "EVENT_2D", "Source" : "cbm9", "Schema": "ev44", "Width": 512, "Height": 512 },
    { "FEN": 3, "Channel": 3, "Type": "EVENT_0D", "Source" : "cbm10", "Schema": "da00", "PixelOffset": 0 },
    { "FEN": 3, "Channel": 4, "Type": "IBM", "Source" : "cbm11", "Schema": "ev44", "MaxTofBin": 10000, "BinCount": 100 }
  ]
}
)"_json;

// clang-format on

using namespace fbserializer;

using HistogramSerializer_t = SchemaDetails::DA00Serializer_t;

class MockHistogramSerializer : public HistogramSerializer_t {
public:
  MOCK_METHOD(void, addEvent, (const int32_t &time, const int32_t &data),
              (override));

  MockHistogramSerializer()
      : HistogramSerializer("cbm", 10, 10, "A", BinningStrategy::LastBin) {}
};

class CbmInstrumentTest : public TestBase {
public:
protected:
  /// Objects required to build the CbmInstrument
  struct Counters CbmCounters;
  BaseSettings Settings;
  std::unique_ptr<Config> Configuration;
  TestHeaderFactory headerFactory;

  /// Objects required for the CbmInstrument
  /// \note These are initialized in SetUp()
  std::unique_ptr<Statistics> Stats;
  std::unique_ptr<ESSReadout::Parser> ESSHeaderParser;
  std::unique_ptr<cbm::Parser> CbmReadoutParser;
  HashMap2D<SchemaDetails> SchemaMap{11};
  std::unique_ptr<CbmInstrument> cbm;

  inline static path FullConfigFile{""};

  void SetUp() override {
    // Initialize stats and parser
    Stats = std::make_unique<Statistics>();
    ESSHeaderParser = std::make_unique<ESSReadout::Parser>(*Stats);
    CbmReadoutParser = std::make_unique<cbm::Parser>();
    // Reinitialize Configuration as unique_ptr
    Configuration = std::make_unique<Config>();
    Configuration->setRoot(TestConfig);
    Configuration->apply();

    initializeSerializers();
    initializeCbmInstrument();

    CbmCounters = {};

    ESSHeaderParser->Packet.HeaderPtr =
        headerFactory.createHeader(ESSReadout::Parser::V1);
  }

  void TearDown() override {}

protected:
  void makeHeader(ESSReadout::Parser::PacketDataV0 &Packet,
                  std::vector<uint8_t> &testdata) {
    Packet.HeaderPtr = headerFactory.createHeader(ESSReadout::Parser::V1);
    Packet.DataPtr = (char *)&testdata[0];
    Packet.DataLength = testdata.size();
    Packet.Time.setReference(ESSTime(0, 0));
    Packet.Time.setPrevReference(ESSTime(0, 0));
  }

  void initializeSerializers() {
    for (auto &Topology : Configuration->TopologyMapPtr->toValuesList()) {
      std::unique_ptr<SchemaDetails> details;

      if (Topology->Type == CbmType::EVENT_0D) {
        if (Topology->Schema == SchemaType::EV44) {

          std::unique_ptr<EV44Serializer> Serializer =
              std::make_unique<EV44SerializerMock>();
          details = std::make_unique<SchemaDetails>(Topology->Schema,
                                                    std::move(Serializer));
        } else if (Topology->Schema == SchemaType::DA00) {

          std::unique_ptr<HistogramSerializer_t> Serializer =
              std::make_unique<MockHistogramSerializer>();
          details = std::make_unique<SchemaDetails>(Topology->Schema,
                                                    std::move(Serializer));
        } else {
          FAIL() << "Schema configuration is not correct";
        }

      } else if (Topology->Type == CbmType::EVENT_2D) {

        std::unique_ptr<EV44Serializer> Serializer =
            std::make_unique<EV44SerializerMock>();
        details = std::make_unique<SchemaDetails>(Topology->Schema,
                                                  std::move(Serializer));

      } else if (Topology->Type == CbmType::IBM) {

        if (Topology->Schema == SchemaType::EV44) {

          std::unique_ptr<EV44Serializer> Serializer =
              std::make_unique<EV44SerializerMock>();
          details = std::make_unique<SchemaDetails>(Topology->Schema,
                                                    std::move(Serializer));
        } else if (Topology->Schema == SchemaType::DA00) {

          std::unique_ptr<HistogramSerializer_t> Serializer =
              std::make_unique<MockHistogramSerializer>();
          details = std::make_unique<SchemaDetails>(Topology->Schema,
                                                    std::move(Serializer));
        } else {
          FAIL() << "Schema configuration is not correct";
        }
      }
      SchemaMap.add(Topology->FEN, Topology->Channel, details);
    }
  }

  void initializeCbmInstrument() {
    cbm = std::make_unique<CbmInstrument>(*Stats, CbmCounters, *Configuration,
                                          *CbmReadoutParser, SchemaMap,
                                          *ESSHeaderParser);
  }
};

///
/// \brief Test case for validating Event0D type readouts.
///
/// Verifies schema routing: EV44 and DA00 serializers receive correct events.
/// Time and geometry calculations are tested in ESSTime and Geometry tests.
///
TEST_F(CbmInstrumentTest, TestEvent0DSchemas) {

  // =========================================================================
  // Set Mock Expectations - EV44 Schema (FEN 0, Ch 0)
  // =========================================================================
  SchemaDetails *details = SchemaMap.get(0, 0);
  EV44SerializerMock *Serializer1 = dynamic_cast<EV44SerializerMock *>(
      details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer1, addEvent(_, _)).Times(1);

  // =========================================================================
  // Set Mock Expectations - DA00 Schema (FEN 3, Ch 3)
  // =========================================================================
  details = SchemaMap.get(3, 3);
  MockHistogramSerializer *Serializer2 =
      dynamic_cast<MockHistogramSerializer *>(
          details->GetSerializer<SchemaType::DA00>());
  EXPECT_CALL(*Serializer2, addEvent(_, 1)).Times(1);

  // =========================================================================
  // Initialize Test Data
  // =========================================================================
  makeHeader(ESSHeaderParser->Packet, ValidEvent0DReadouts);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 2);
  EXPECT_EQ(CbmCounters.CbmStats.Readouts0D, 2);
  EXPECT_EQ(CbmCounters.CbmStats.Readouts2D, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ReadoutsIBM, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.CbmCounts, 2);
  EXPECT_EQ(CbmCounters.Event0DEvents, 2);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
}

///
/// \brief Test case for validating Event2D readouts.
///
/// Verifies EV44 schema routing (only schema available for EVENT_2D).
/// Pixel calculations are tested in Geometry tests.
///
TEST_F(CbmInstrumentTest, TestEvent2DSchemas) {

  // =========================================================================
  // Set Mock Expectations - EV44 Schema (FEN 3, Ch 0)
  // =========================================================================
  SchemaDetails *details = SchemaMap.get(3, 0);
  EV44SerializerMock *Serializer = dynamic_cast<EV44SerializerMock *>(
      details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer, addEvent(_, _)).Times(1);

  // =========================================================================
  // Initialize Test Data
  // =========================================================================
  makeHeader(ESSHeaderParser->Packet, ValidEvent2DReadouts);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 1);
  EXPECT_EQ(CbmCounters.CbmStats.Readouts2D, 1);
  EXPECT_EQ(CbmCounters.CbmStats.Readouts0D, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ReadoutsIBM, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.CbmCounts, 1);
  EXPECT_EQ(CbmCounters.Event2DEvents, 1);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
}

///
/// \brief Test case for validating IBM type readouts.
///
/// Verifies schema routing: DA00 and EV44 serializers receive correct events.
/// NPOS value handling and normalization are tested separately.
///
TEST_F(CbmInstrumentTest, TestIBMSchemas) {

  // =========================================================================
  // Set Mock Expectations - DA00 Schema (FEN 1, Ch 1)
  // =========================================================================
  SchemaDetails *details = SchemaMap.get(1, 1);
  MockHistogramSerializer *Serializer1 =
      dynamic_cast<MockHistogramSerializer *>(
          details->GetSerializer<SchemaType::DA00>());
  EXPECT_CALL(*Serializer1, addEvent(_, _)).Times(1);

  // =========================================================================
  // Set Mock Expectations - EV44 Schema (FEN 3, Ch 4)
  // =========================================================================
  details = SchemaMap.get(3, 4);
  EV44SerializerMock *Serializer2 = dynamic_cast<EV44SerializerMock *>(
      details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer2, addEvent(_, _)).Times(1);

  // =========================================================================
  // Initialize Test Data
  // =========================================================================
  makeHeader(ESSHeaderParser->Packet, ValidIBMReadouts);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 2);
  EXPECT_EQ(CbmCounters.CbmStats.ReadoutsIBM, 2);
  EXPECT_EQ(CbmCounters.CbmStats.Readouts0D, 0);
  EXPECT_EQ(CbmCounters.CbmStats.Readouts2D, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.CbmCounts, 2);
  EXPECT_EQ(CbmCounters.IBMEvents, 2);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);
}

///
/// \brief Test that validation errors are properly accumulated across all types
///
/// This test verifies that when different readout types fail validation,
/// errors are counted in a single unified geometry - not separate counters
/// per type that could shadow each other.
///
TEST_F(CbmInstrumentTest, TestValidationErrorsAcrossTypes) {

  // =========================================================================
  // Set Mock Expectations - No events should be added (all fail validation)
  // =========================================================================
  SchemaDetails *details = SchemaMap.get(0, 0);
  EV44SerializerMock *Serializer1 = dynamic_cast<EV44SerializerMock *>(
      details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer1, addEvent(_, _)).Times(0);

  details = SchemaMap.get(3, 0);
  EV44SerializerMock *Serializer2 = dynamic_cast<EV44SerializerMock *>(
      details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer2, addEvent(_, _)).Times(0);

  details = SchemaMap.get(1, 1);
  MockHistogramSerializer *Serializer3 =
      dynamic_cast<MockHistogramSerializer *>(
          details->GetSerializer<SchemaType::DA00>());
  EXPECT_CALL(*Serializer3, addEvent(_, _)).Times(0);

  // =========================================================================
  // Initialize Test Data - Mixed types with different validation failures
  // =========================================================================
  makeHeader(ESSHeaderParser->Packet, ValidationErrorsAcrossTypes);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  // check parser counters are as expected
  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 3);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorType, 0);

  // =========================================================================
  // Process and Verify Results
  // =========================================================================
  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.CbmCounts, 0); // No valid events (all fail validation)
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);

  // Verify all validation errors are accumulated in a single geometry instance
  // Not in separate per-type counters
  const cbm::Geometry &geometry = cbm->GetGeometry();
  // MonitorRingMismatch: EVENT_0D (Ring 10) + IBM (Ring 9) = 2
  EXPECT_EQ(geometry.getCbmCounters().MonitorRingMismatchErrors, 2);
  // TopologyError: EVENT_2D (FEN 3, Ch 5 not in config) = 1
  EXPECT_EQ(geometry.getBaseCounters().TopologyError, 1);
  // Total validation errors across all types
  EXPECT_EQ(geometry.getBaseCounters().ValidationErrors, 3);
}

///
/// \brief Test IBM readout normalization and NPOSCount accumulation
///
/// Verifies that with normalization enabled:
/// - NADC values are divided by MCASum when MCASum != 0
/// - When MCASum = 0, raw NADC value is used (division skipped)
/// - NPOSCount accumulates the (normalized or raw) values across all readouts
///
/// Test data:
/// - Readout 1: NADC = 4096, MCASum = 2 -> normalized = 2048
/// - Readout 2: NADC = 8192, MCASum = 4 -> normalized = 2048
/// - Readout 3: NADC = 4096, MCASum = 0 -> raw value 4096 (division skipped)
/// - Expected NPOSCount = 2048 + 2048 + 4096 = 8192
TEST_F(CbmInstrumentTest, TestIBMNormalizeReadouts) {

  // =========================================================================
  // Set Mock Expectations - Normalized and raw values
  // =========================================================================
  int FenId = 2;
  int ChannelId = 1;
  SchemaDetails *details = SchemaMap.get(FenId, ChannelId);
  MockHistogramSerializer *Serializer = dynamic_cast<MockHistogramSerializer *>(
      details->GetSerializer<SchemaType::DA00>());

  // Readout 1: NADC=4096/MCASum=2 = 2048 normalized
  // Readout 2: NADC=8192/MCASum=4 = 2048 normalized
  // Readout 3: NADC=4096, MCASum=0 -> raw 4096 (division skipped)
  EXPECT_CALL(*Serializer, addEvent(_, Eq(2048))).Times(2);
  EXPECT_CALL(*Serializer, addEvent(_, Eq(4096))).Times(1);

  makeHeader(ESSHeaderParser->Packet, IBMNormalizeReadouts);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 3);
  EXPECT_EQ(CbmCounters.CbmStats.ReadoutsIBM, 3);

  cbm->processMonitorReadouts();

  EXPECT_EQ(CbmCounters.IBMEvents, 3);
  // NPOSCount should be sum: 2048 + 2048 + 4096 = 8192
  EXPECT_EQ(CbmCounters.NPOSCount, 8192);
}

///
/// \brief Test IBM readout with normalization disabled and NPOSCount
/// accumulation
///
/// Verifies that with normalization disabled:
/// - Raw NADC values are used (not divided by MCASum)
/// - NPOSCount accumulates the raw NADC values
///
/// Test data (same as enabled test):
/// - Readout 1: NADC = 4096 (raw, not normalized)
/// - Readout 2: NADC = 8192 (raw, not normalized)
/// - Readout 3: NADC = 4096, MCASum = 0 (raw, same as enabled)
/// - Expected NPOSCount = 4096 + 8192 + 4096 = 16384
TEST_F(CbmInstrumentTest, TestIBMDisableNormalizeReadouts) {

  // =========================================================================
  // Test Setup - Disable normalization
  // =========================================================================
  this->Configuration->CbmParms.NormalizeIBMReadouts = false;

  // =========================================================================
  // Set Mock Expectations - Raw NADC values (no division)
  // =========================================================================
  int FenId = 2;
  int ChannelId = 1;
  SchemaDetails *details = SchemaMap.get(FenId, ChannelId);
  MockHistogramSerializer *Serializer = dynamic_cast<MockHistogramSerializer *>(
      details->GetSerializer<SchemaType::DA00>());

  // Expect raw NADC values: 4096 (x2) and 8192
  EXPECT_CALL(*Serializer, addEvent(_, Eq(4096))).Times(2);
  EXPECT_CALL(*Serializer, addEvent(_, Eq(8192))).Times(1);

  makeHeader(ESSHeaderParser->Packet, IBMNormalizeReadouts);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 3);
  EXPECT_EQ(CbmCounters.CbmStats.ReadoutsIBM, 3);

  cbm->processMonitorReadouts();

  EXPECT_EQ(CbmCounters.IBMEvents, 3);
  // NPOSCount should be sum of raw NADC values: 4096 + 8192 + 4096 = 16384
  EXPECT_EQ(CbmCounters.NPOSCount, 16384);

  // =========================================================================
  // Cleanup - Rollback normalize flag changes
  // =========================================================================
  this->Configuration->CbmParms.NormalizeIBMReadouts = true;
}

///
/// \brief Test case for validating topology and type error detection.
///
/// This test verifies two unreachable error paths in the instrument:
///
/// 1. NoSerializerCfgError: When FEN/Channel combinations are not in the
///    topology configuration, geometry validation catches the error BEFORE
///    attempting to access the serializer.
///
/// 2. TypeNotConfigured: Invalid types (not 1, 2, or 3) are caught by the
///    parser via ErrorType counter and dropped before reaching the instrument.
///
TEST_F(CbmInstrumentTest, TestProcessingErrorPrevention) {
  makeHeader(ESSHeaderParser->Packet, FenAndChannelNotInCfgReadout);

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  // Parser sees 4 readouts: 2 invalid FEN/Ch, 1 valid, 1 invalid type
  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 4);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorADC, 0);
  // Invalid type (Type 4) caught by parser
  EXPECT_EQ(CbmCounters.CbmStats.ErrorType, 1);

  cbm->processMonitorReadouts();
  // Only 1 valid readout passes validation and produces an event
  EXPECT_EQ(CbmCounters.CbmCounts, 1);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 1); // The valid one produces an event
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);

  // Verify that validation catches topology errors
  // 2 readouts have invalid FEN/Channel combinations
  const auto &geom = cbm->GetGeometry();
  EXPECT_EQ(geom.getBaseCounters().TopologyError, 2);
  EXPECT_EQ(geom.getBaseCounters().ValidationErrors, 2);

  // Verify that NoSerializerCfgError does NOT occur because validation
  // prevents the code from ever reaching SchemaMap.get()
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);

  // Verify that TypeNotConfigured does NOT occur because parser drops
  // invalid types before they reach processMonitorReadouts()
  EXPECT_EQ(CbmCounters.TypeNotConfigured, 0);

  // Only 1 readout reaches TOF calculation (the valid one)
  EXPECT_EQ(ESSHeaderParser->Packet.Time.Counters.TofCount, 1);
}

///
/// \brief Test case for the scenario when the calculated TOF is higher then the
/// MaxTof limit configured in the configuration file
///
TEST_F(CbmInstrumentTest, HighTofErrorDefaultValue) {
  // =========================================================================
  // Set Mock Expectations - No events should be added due to high TOF
  // =========================================================================
  SchemaDetails *details = SchemaMap.get(0, 0);
  EV44SerializerMock *Serializer = dynamic_cast<EV44SerializerMock *>(
      details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer, addEvent(_, _)).Times(0);

  makeHeader(ESSHeaderParser->Packet, TofToHighReadout);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 0));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(0, 1000000));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  cbm->processMonitorReadouts();

  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 1);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorType, 0);

  EXPECT_EQ(CbmCounters.CbmCounts, 0);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);

  // Verify that getTOF detected high TOF
  EXPECT_EQ(ESSHeaderParser->Packet.Time.Counters.TofHigh, 1);
  EXPECT_EQ(ESSHeaderParser->Packet.Time.Counters.TofCount, 0);
}

///
/// \brief Test successful fallback to previous pulse time for TOF calculation
///
/// When readout time is between PrevPulseTime and PulseTime (negative TOF vs
/// current), the system falls back to using PrevPulseTime for TOF calculation.
///
/// Readout: Time = 2s + 90000 ticks
/// PulseTime = 2s + 100000 ticks (readout is before this → negative TOF)
/// PrevPulse = 2s + 50000 ticks (readout is after this → positive prev TOF)
///
TEST_F(CbmInstrumentTest, TestPreTofFallback) {
  // =========================================================================
  // Set Mock Expectations - Event should be created using prev pulse TOF
  // =========================================================================
  SchemaDetails *details = SchemaMap.get(0, 0);
  EV44SerializerMock *Serializer = dynamic_cast<EV44SerializerMock *>(
      details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer, addEvent(Ge(0), Ge(0))).Times(1);

  makeHeader(ESSHeaderParser->Packet, TimingErrorReadout);
  // Readout at 2s + 90000 is between PrevPulse (2s + 50000) and PulseTime (2s +
  // 100000)
  ESSHeaderParser->Packet.Time.setReference(ESSTime(2, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(2, 50000));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 1);

  cbm->processMonitorReadouts();

  // Event should be created using previous pulse TOF
  EXPECT_EQ(CbmCounters.CbmCounts, 1);
  EXPECT_EQ(CbmCounters.Event0DEvents, 1);

  // Verify TOF fallback counters
  EXPECT_EQ(ESSHeaderParser->Packet.Time.Counters.TofNegative, 1);
  EXPECT_EQ(ESSHeaderParser->Packet.Time.Counters.PrevTofCount, 1);
  EXPECT_EQ(ESSHeaderParser->Packet.Time.Counters.PrevTofNegative, 0);
}

///
/// \brief Test negative previous TOF error (readout before PrevPulseTime)
///
/// When readout time is before both PulseTime and PrevPulseTime,
/// the event is discarded due to negative prev TOF.
///
/// Readout: Time = 2s + 90000 ticks
/// PulseTime = 3s + 100000 ticks (readout way before → negative TOF)
/// PrevPulse = 2s + 100000 ticks (readout before this too → negative prev TOF)
///
TEST_F(CbmInstrumentTest, TestNegativePrevTofError) {
  // =========================================================================
  // Set Mock Expectations - No event should be created
  // =========================================================================
  SchemaDetails *details = SchemaMap.get(0, 0);
  EV44SerializerMock *Serializer = dynamic_cast<EV44SerializerMock *>(
      details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer, addEvent(_, _)).Times(0);

  makeHeader(ESSHeaderParser->Packet, TimingErrorReadout);
  // Readout at 2s + 90000 is before PrevPulse (2s + 100000)
  ESSHeaderParser->Packet.Time.setReference(ESSTime(3, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(2, 100000));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 1);

  cbm->processMonitorReadouts();

  // No event created - discarded due to negative prev TOF
  EXPECT_EQ(CbmCounters.CbmCounts, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);

  // Verify negative prev TOF was detected
  EXPECT_EQ(ESSHeaderParser->Packet.Time.Counters.TofNegative, 1);
  EXPECT_EQ(ESSHeaderParser->Packet.Time.Counters.PrevTofCount, 0);
  EXPECT_EQ(ESSHeaderParser->Packet.Time.Counters.PrevTofNegative, 1);
}

int main(int argc, char **argv) {
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
