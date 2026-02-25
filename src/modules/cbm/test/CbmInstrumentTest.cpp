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
#include <geometry/Geometry0D.h>
#include <geometry/Geometry2D.h>
#include <memory>
#include <modules/cbm/CbmInstrument.h>

using namespace cbm;
using namespace ESSReadout;
using namespace testing;

using std::filesystem::path;
using ESSParser = ESSReadout::Parser;

// clang-format off

/// \brief Monitor readout with valid Event0D readouts with time values
std::vector<uint8_t> ValidEvent0DReadouts {
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA2, 0x86, 0x01, 0x00,  // Time LO 100002 tick
  0x01, 0x01, 0x01, 0x00,  // Type 1, Ch 1, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  0x16, 0x01, 0x14, 0x00,  // Fiber 22, FEN 1, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA3, 0x86, 0x01, 0x00,  // Time LO 100003 tick
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

/// \brief Monitor readout with valid Event2D readouts with time values
std::vector<uint8_t> ValidEvent2DReadouts {
  0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
  0x02, 0x00, 0x01, 0x00,  // Type 2, Ch 0, ADC 1
  0x00, 0x01, 0x01, 0x01,  // XPos 256, YPos 257

  0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA2, 0x86, 0x01, 0x00,  // Time LO 100002 tick
  0x02, 0x01, 0x01, 0x00,  // Type 2, Ch 1, ADC 1
  0xAA, 0x01, 0xBB, 0x01,  // XPos 426, YPos 443

  0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA3, 0x86, 0x01, 0x00,  // Time LO 100003 tick
  0x02, 0x02, 0x01, 0x00,  // Type 2, Ch 2, ADC 1
  0xCC, 0x00, 0xDD, 0x00   // XPos 204, YPos 221
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

/// \brief Monitor readout with valid IBM readouts with time and NPOS values
std::vector<uint8_t> ValidIBMReadouts {
  // Test low 8bit NPOS value
  0x16, 0x01, 0x14, 0x00,  // Fiber 22, FEN 1, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
  0x03, 0x01, 0x00, 0x01,  // Type 3, Ch 1, ADC 1
  0x0A, 0x00, 0x00, 0x00,  // NPOS 10

  // Test medium 16bit NPOS value
  0x16, 0x01, 0x14, 0x00,  // Fiber 22, FEN 1, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA2, 0x86, 0x01, 0x00,  // Time LO 100002 tick
  0x03, 0x02, 0x01, 0x00,  // Type 3, Ch 2, ADC 1
  0xF4, 0x01, 0x00, 0x00,  // NPOS 500

    // Test high 24bit NPOS value
  0x16, 0x02, 0x14, 0x00,  // Fiber 22, FEN 2, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA3, 0x86, 0x01, 0x00,  // Time LO 100003 tick
  0x03, 0x01, 0x01, 0x00,  // Type 3, Ch 1, ADC 1
  0xFF, 0xFF, 0xFF, 0x00,   // NADC (3 bytes), MCA Sum (0)

  // Test high 32bit NPOS value, should not be accepted
  0x16, 0x02, 0x14, 0x00,  // Fiber 22, FEN 2, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA3, 0x86, 0x01, 0x00,  // Time LO 100003 tick
  0x03, 0x01, 0x01, 0x00,  // Type 3, Ch 1, ADC 1
  0xFF, 0xFF, 0xFF, 0x01   // NADC (3 bytes), MCA Sum (1)
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

/// \brief Monitor readout with not supported Type
std::vector<uint8_t> NotSupportedTypeReadout {
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x04, 0x00, 0x00, 0x00,  // Type 4, Ch 0, ADC 0
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0
};

/// \brief Monitor readout with invalid FEN and Channel
std::vector<uint8_t> FenAndChannelNotInCfgReadout {
  // Invalid FEN - not in configuration
  0x16, 0x06, 0x14, 0x00,  // Fiber 22, FEN 6, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,   // XPos 0, YPos 0

  // Invalid Channel - not in configuration
  0x16, 0x06, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 14, ADC 1
  0x00, 0x00, 0x00, 0x00,   // XPos 0, YPos 0

  // Invalid Ring and Channel - not in configuration
  0x16, 0x06, 0x14, 0x00,  // Fiber 22, FEN 6, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 14, ADC 1
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

/// \brief Monitor readout to test all negative TOF scenarios
std::vector<uint8_t> PreviousAndNegativePrevTofReadouts {
  // First Readout - TOF between PrevPulse and PulseTime
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x02, 0x00, 0x00, 0x00,  // Time HI 2 s
  0x09, 0x5F, 0x01, 0x00,  // Time LO 90000 tick
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,   // XPos 0, YPos 0

  // Second Readout - TOF before PrevPulseTime
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
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
  "MaxFENId" : 3,
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

class MockHistogramSerializer
    : public HistogramSerializer_t {
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
          details = std::make_unique<SchemaDetails>(
            Topology->Schema, std::move(Serializer));
        } else if (Topology->Schema == SchemaType::DA00) {

          std::unique_ptr<HistogramSerializer_t> Serializer =
            std::make_unique<MockHistogramSerializer>();
          details = std::make_unique<SchemaDetails>(
            Topology->Schema, std::move(Serializer));
        } else {
          FAIL() << "Schema configuration is not correct";
        }

      } else if (Topology->Type == CbmType::EVENT_2D) {

        std::unique_ptr<EV44Serializer> Serializer =
          std::make_unique<EV44SerializerMock>();
        details = std::make_unique<SchemaDetails>(
          Topology->Schema, std::move(Serializer));
        
      } else if (Topology->Type == CbmType::IBM) {

        if (Topology->Schema == SchemaType::EV44) {

          std::unique_ptr<EV44Serializer> Serializer =
            std::make_unique<EV44SerializerMock>();
          details = std::make_unique<SchemaDetails>(
            Topology->Schema, std::move(Serializer));
        } else if (Topology->Schema == SchemaType::DA00) {

          std::unique_ptr<HistogramSerializer_t> Serializer =
            std::make_unique<MockHistogramSerializer>();
          details = std::make_unique<SchemaDetails>(
            Topology->Schema, std::move(Serializer));
        } else {
          FAIL() << "Schema configuration is not correct";
        }
      }
      SchemaMap.add(Topology->FEN, Topology->Channel, details);
    }
  }

  void initializeCbmInstrument() {
    cbm = std::make_unique<CbmInstrument>(
        *Stats, CbmCounters, *Configuration, *CbmReadoutParser,
        SchemaMap, *ESSHeaderParser);
  }
};

/// \brief Test case for validating IBM type readouts serialized into a
/// ev44 schema
///TestEvent0DToDA00Schema
TEST_F(CbmInstrumentTest, TestIBMToEV44Schema) {

  // =========================================================================
  // Set Mock Expectations
  // =========================================================================
  // Serializer 1
  SchemaDetails *details = SchemaMap.get(3, 4);
  EV44SerializerMock *Serializer =
      dynamic_cast<EV44SerializerMock *>(
        details->GetSerializer<SchemaType::EV44>());
  int expectedTime = 1 * ESSTime::ESSClockTick;
  int expectedData = 10;
  EXPECT_CALL(*Serializer,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  expectedTime = 2 * ESSTime::ESSClockTick;
  expectedData = 500;
  EXPECT_CALL(*Serializer,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  expectedTime = 3 * ESSTime::ESSClockTick;
  expectedData = 16777215;
  EXPECT_CALL(*Serializer,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  // =========================================================================
  // Initialize Test Data
  // =========================================================================
  /// \brief Monitor readout with valid IBM readouts with time and NPOS values
  std::vector<uint8_t> DataPacket {
    // Test low 8bit NPOS value
    0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
    0x03, 0x04, 0x00, 0x01,  // Type 3, Ch 4, ADC 1
    0x0A, 0x00, 0x00, 0x00,  // NPOS 10

    // Test medium 16bit NPOS value
    0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0xA2, 0x86, 0x01, 0x00,  // Time LO 100002 tick
    0x03, 0x04, 0x01, 0x00,  // Type 3, Ch 4, ADC 1
    0xF4, 0x01, 0x00, 0x00,  // NPOS 500

      // Test high 24bit NPOS value
    0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0xA3, 0x86, 0x01, 0x00,  // Time LO 100003 tick
    0x03, 0x04, 0x01, 0x00,  // Type 3, Ch 4, ADC 1
    0xFF, 0xFF, 0xFF, 0x00,   // NADC (3 bytes), MCA Sum (0)

    // Test high 32bit NPOS value, should not be accepted
    0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0xA3, 0x86, 0x01, 0x00,  // Time LO 100003 tick
    0x03, 0x04, 0x01, 0x00,  // Type 3, Ch 4, ADC 1
    0xFF, 0xFF, 0xFF, 0x01   // NADC (3 bytes), MCA Sum (1)
  };

  makeHeader(ESSHeaderParser->Packet, DataPacket);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  // check parser counters are as expected
  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 4);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorType, 0);

  // check monitor readouts are processed correctly
  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.CbmCounts, 4);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 4);
  EXPECT_EQ(CbmCounters.IBMEvents, 4);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);
  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_TOF_COUNT),
            4);
}

///
/// \brief Test case for Event0D type readouts serialized with
/// DA00 schema.
///
TEST_F(CbmInstrumentTest, TestEvent0DToDA00Schema) {
  // =========================================================================
  // Set Mock Expectations
  // =========================================================================
  // Serializer 1
  int expectedTime = 1 * ESSTime::ESSClockTick;
  int expectedData = 1;
  SchemaDetails *details = SchemaMap.get(3, 3);

  MockHistogramSerializer *Serializer =
      dynamic_cast<MockHistogramSerializer *>(
        details->GetSerializer<SchemaType::DA00>());
  EXPECT_CALL(*Serializer,
    addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  expectedTime = 2 * ESSTime::ESSClockTick;
  expectedData = 1;
  EXPECT_CALL(*Serializer,
    addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  expectedTime = 3 * ESSTime::ESSClockTick;
  expectedData = 1;
  EXPECT_CALL(*Serializer,
    addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  // =========================================================================
  // Initialize Test Data
  // =========================================================================

  /// Monitor readout with valid Event0D readouts with time values
  std::vector<uint8_t> DataPacket {
    0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
    0x01, 0x03, 0x01, 0x00,  // Type 1, Ch 3, ADC 1
    0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

    0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0xA2, 0x86, 0x01, 0x00,  // Time LO 100002 tick
    0x01, 0x03, 0x01, 0x00,  // Type 1, Ch 3, ADC 1
    0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

    0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0xA3, 0x86, 0x01, 0x00,  // Time LO 100003 tick
    0x01, 0x03, 0x01, 0x00,  // Type 1, Ch 3, ADC 1
    0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
  };

  makeHeader(ESSHeaderParser->Packet, DataPacket);
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

  // check monitor readouts are processed correctly
  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.CbmCounts, 3);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 3);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 3);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);
  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_TOF_COUNT),
            3);
}

///
/// \brief Test case for validating Event0D type readouts.
///
/// This test case sets expectations on the mocked serializer objects and checks
/// if the monitor readouts are processed correctly. It verifies the parser
/// counters and the processed readout counts. Also test that the serializer's
/// addEvent is called with proper arguments.
///
TEST_F(CbmInstrumentTest, TestValidEvent0DReadouts) {

  // =========================================================================
  // Set Mock Expectations
  // =========================================================================
  // Serializer 1
  int expectedTime = 1 * ESSTime::ESSClockTick;
  int expectedData = 0;
  SchemaDetails *details = SchemaMap.get(0, 0);

  EV44SerializerMock *Serializer1 =
      dynamic_cast<EV44SerializerMock *>(
        details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer1,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  expectedTime = 2 * ESSTime::ESSClockTick;
  expectedData = 1;
  details = SchemaMap.get(0, 1);
  EV44SerializerMock *Serializer2 =
      dynamic_cast<EV44SerializerMock *>(
        details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer2,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  expectedTime = 3 * ESSTime::ESSClockTick;
  expectedData = 3;
  details = SchemaMap.get(1, 0);
  EV44SerializerMock *Serializer3 =
      dynamic_cast<EV44SerializerMock *>(
        details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer3,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  // =========================================================================
  // Initialize Test Data
  // =========================================================================
  makeHeader(ESSHeaderParser->Packet, ValidEvent0DReadouts);
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

  // check monitor readouts are processed correctly
  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.CbmCounts, 3);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 3);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 3);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);
  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_TOF_COUNT),
            3);
}

///
/// \brief Test case for validating Event2D readouts using valid data.
///
/// This test case sets expectations on the mocked serializer objects and checks
/// if the monitor readouts are processed correctly. It verifies the parser
/// counters and the processed readout counts. Also test that the serializer's
/// addEvent is called with proper arguments.
///
TEST_F(CbmInstrumentTest, TestValidEvent2DReadouts) {

  const int HorizontalWidth = 512;

  // =========================================================================
  // Set Mock Expectations
  // =========================================================================
  // Serializer 1 - Test pixel id calculation
  int expectedTime = 1 * ESSTime::ESSClockTick;
  // Test Data => XPos 256, YPos 257
  int expectedData = 257 * HorizontalWidth + 256 + 1;
  SchemaDetails *details = SchemaMap.get(3, 0);

  EV44SerializerMock *Serializer1 =
      dynamic_cast<EV44SerializerMock *>(
        details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer1,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  expectedTime = 2 * ESSTime::ESSClockTick;
  // Test Data => XPos 426, YPos 443
  expectedData = 443 * HorizontalWidth + 426 + 1;
  details = SchemaMap.get(3, 1);
  EV44SerializerMock *Serializer2 =
      dynamic_cast<EV44SerializerMock *>(
        details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer2,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  expectedTime = 3 * ESSTime::ESSClockTick;
  // Test Data => XPos 204, YPos 221
  expectedData = 221 * HorizontalWidth + 204 + 1;
  details = SchemaMap.get(3, 2);
  EV44SerializerMock *Serializer3 =
      dynamic_cast<EV44SerializerMock *>(
        details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer3,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  // =========================================================================
  // Initialize Test Data
  // =========================================================================
  makeHeader(ESSHeaderParser->Packet, ValidEvent2DReadouts);
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
  EXPECT_EQ(CbmCounters.CbmCounts, 3);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 3);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 3);
  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_TOF_COUNT),
            3);
}

///
/// \brief Test case for validating Event2D type readouts using invalid data.
///
/// This test case sets expectations on the mocked serializer objects and checks
/// if the monitor readouts are processed correctly. It verifies the parser
/// counters and the processed readout counts. Also test that the serializer's
/// addEvent is called with proper arguments.
///
TEST_F(CbmInstrumentTest, TestInvalidEvent2DReadouts) {

  // =========================================================================
  // Set Mock Expectations - No events should be added due to invalid positions
  // =========================================================================
  SchemaDetails *details = SchemaMap.get(3, 0);

  EV44SerializerMock *Serializer1 =
      dynamic_cast<EV44SerializerMock *>(
        details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer1, addEvent(_, _)).Times(0);

  details = SchemaMap.get(3, 1);
  EV44SerializerMock *Serializer2 =
      dynamic_cast<EV44SerializerMock *>(
        details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer2, addEvent(_, _)).Times(0);

  details = SchemaMap.get(3, 2);
  EV44SerializerMock *Serializer3 =
      dynamic_cast<EV44SerializerMock *>(
        details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer3, addEvent(_, _)).Times(0);

  // =========================================================================
  // Initialize Test Data
  // =========================================================================
  makeHeader(ESSHeaderParser->Packet, InvalidPosEvent2DReadouts);
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
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 3);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0); // No events (all fail validation)

  // Verify validation errors are tracked via Stats

  // Verify X/Y position errors are tracked (2 XPos errors, 1 YPos error due to
  // short-circuit)
  EXPECT_EQ(Stats->getValueByName("cbm7." + Geometry2D::METRIC_XPOS_ERRORS), 1);
  EXPECT_EQ(Stats->getValueByName("cbm8." + Geometry2D::METRIC_YPOS_ERRORS), 1);
  EXPECT_EQ(Stats->getValueByName("cbm9." + Geometry2D::METRIC_XPOS_ERRORS), 1);

  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_TOF_COUNT),
            3);
}

///
/// \brief Test case for validating Event0D type readouts using invalid Ring.
///
/// This test verifies that readouts with Ring values exceeding MaxRing are
/// properly rejected by the geometry validation. Ring errors should be counted
/// and no events should be created.
///
TEST_F(CbmInstrumentTest, TestInvalidEvent0DReadouts) {

  // =========================================================================
  // Set Mock Expectations - No events should be added
  // =========================================================================
  SchemaDetails *details = SchemaMap.get(0, 0);

  EV44SerializerMock *Serializer1 =
    dynamic_cast<EV44SerializerMock *>(
      details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer1, addEvent(_, _)).Times(0);

  details = SchemaMap.get(0, 1);

  EV44SerializerMock *Serializer2 =
    dynamic_cast<EV44SerializerMock *>(
      details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer2, addEvent(_, _)).Times(0);

  // =========================================================================
  // Initialize Test Data
  // =========================================================================
  makeHeader(ESSHeaderParser->Packet, InvalidRingEvent0DReadouts);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  // check parser counters are as expected
  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 2);
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
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 2);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0); // No events (all fail validation)
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);

  // Verify validation errors are tracked via Stats
  //Check cbm1
  const cbm::Geometry *geometry = cbm->GetGeometry(0, 0);
  EXPECT_EQ(geometry->getGeometryCounters().MonitorRingMismatchErrors, 1);

  //Check cbm2
  geometry = cbm->GetGeometry(0, 1);
  EXPECT_EQ(geometry->getGeometryCounters().MonitorRingMismatchErrors, 1);

  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_TOF_COUNT),
            2);
}

///
/// \brief Test case for validating IBM type readouts using invalid Ring.
///
/// This test verifies that readouts with Ring values exceeding MaxRing are
/// properly rejected by the geometry validation. Ring errors should be counted
/// and no events should be created.
///
TEST_F(CbmInstrumentTest, TestInvalidIBMReadouts) {

  // =========================================================================
  // Set Mock Expectations - No events should be added
  // =========================================================================
  SchemaDetails *details = SchemaMap.get(1, 1);
  MockHistogramSerializer *Serializer1 =
    dynamic_cast<MockHistogramSerializer *>(
      details->GetSerializer<SchemaType::DA00>());
  EXPECT_CALL(*Serializer1, addEvent(_, _)).Times(0);

  details = SchemaMap.get(2, 1);
  MockHistogramSerializer *Serializer2 =
    dynamic_cast<MockHistogramSerializer *>(
      details->GetSerializer<SchemaType::DA00>());
  EXPECT_CALL(*Serializer2, addEvent(_, _)).Times(0);

  // =========================================================================
  // Initialize Test Data
  // =========================================================================
  makeHeader(ESSHeaderParser->Packet, InvalidRingIBMReadouts);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  // check parser counters are as expected
  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 2);
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
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 2);
  EXPECT_EQ(CbmCounters.IBMEvents, 0); // No events (all fail validation)
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);

  // Verify validation errors are tracked via Stats
  // clang-format off
  // Verify validation errors are tracked via Stats
  //Check cbm4
  const cbm::Geometry *geometry = cbm->GetGeometry(1, 1);
  EXPECT_EQ(geometry->getGeometryCounters().MonitorRingMismatchErrors, 1);

  //Check cbm6
  geometry = cbm->GetGeometry(2, 1);
  EXPECT_EQ(geometry->getGeometryCounters().MonitorRingMismatchErrors, 1);

  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_TOF_COUNT), 2);
  // clang-format on
}

///
/// \brief Test case for validating type readouts.
///
/// This test verifies that readouts with wrong type compared to configuration 
/// generate an type_mismatch_errors metric
///
TEST_F(CbmInstrumentTest, TestInvalidTypeReadouts) {

  // =========================================================================
  // Set Mock Expectations - No events should be added due to type mismatch
  // =========================================================================
  SchemaDetails *details = SchemaMap.get(1, 0); //cbm3 EVENT_0D type 1
  EV44SerializerMock *Serializer1 =
    dynamic_cast<EV44SerializerMock *>(
      details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer1, addEvent(_, _)).Times(0);

  details = SchemaMap.get(2, 1); // //cbm 6 IBM type 3
  MockHistogramSerializer *Serializer2 =
    dynamic_cast<MockHistogramSerializer *>(
      details->GetSerializer<SchemaType::DA00>());
  EXPECT_CALL(*Serializer2, addEvent(_, _)).Times(0);

  details = SchemaMap.get(3, 1); //cbm8 EVENT_2D type 2
  EV44SerializerMock *Serializer3 =
    dynamic_cast<EV44SerializerMock *>(
      details->GetSerializer<SchemaType::EV44>());
  EXPECT_CALL(*Serializer3, addEvent(_, _)).Times(0);

  // =========================================================================
  // Initialize Test Data
  // =========================================================================
  /// \brief Monitor readout with wrong Types using 3 for EVENT_0D, 2 for IBM
  /// and 1 for EVENT_2D
  std::vector<uint8_t> DReadouts {
    0x16, 0x01, 0x14, 0x00,  // Fiber 22, FEN 1, Data Length 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
    0x03, 0x00, 0x01, 0x00,  // Type 3, Ch 0, ADC 1
    0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

    0x16, 0x02, 0x14, 0x00,  // Fiber 22, FEN 2, Data Length 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0xA2, 0x86, 0x01, 0x00,  // Time LO 100002 tick
    0x02, 0x01, 0x01, 0x00,  // Type 2, Ch 1, ADC 1
    0xF4, 0x01, 0x00, 0x00,  // NPOS 500

    0x16, 0x03, 0x14, 0x00,  // Fiber 22, FEN 3, Data Length 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0xA3, 0x86, 0x01, 0x00,  // Time LO 100003 tick
    0x01, 0x01, 0x01, 0x00,  // Type 1, Ch 1, ADC 1
    0xCC, 0x00, 0xDD, 0x00   // XPos 204, YPos 221
  };

  makeHeader(ESSHeaderParser->Packet, DReadouts);
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
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 1);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 1);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 1);
  EXPECT_EQ(CbmCounters.IBMEvents, 0); // No events (all fail validation)
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);

  // Verify validation errors are tracked via Stats
  // clang-format off
  // Verify validation errors are tracked via Stats
  //Check cbm3
  const cbm::Geometry *geometry = cbm->GetGeometry(1, 0);
  EXPECT_EQ(geometry->getGeometryCounters().TypeMismatchError, 1);

  //Check cbm6
  geometry = cbm->GetGeometry(2, 1);
  EXPECT_EQ(geometry->getGeometryCounters().TypeMismatchError, 1);

  //Check cbm8
  geometry = cbm->GetGeometry(3, 1);
  EXPECT_EQ(geometry->getGeometryCounters().TypeMismatchError, 1);
  // clang-format on
}

/// \brief Test case for validating IBM type readouts.
///
/// This test case sets expectations on the mocked serializer objects and checks
/// if the monitor readouts are processed correctly. It verifies the parser
/// counters and the processed readout counts. Also test that the serializer's
/// addEvent is called with proper arguments.
///
TEST_F(CbmInstrumentTest, TestValidIBMReadouts) {

  // =========================================================================
  // Set Mock Expectations
  // =========================================================================
  // Serializer 1
  SchemaDetails *details = SchemaMap.get(1, 1);
  MockHistogramSerializer *Serializer1 =
      dynamic_cast<MockHistogramSerializer *>(
        details->GetSerializer<SchemaType::DA00>());
  int expectedTime = 1 * ESSTime::ESSClockTick;
  int expectedData = 10;
  EXPECT_CALL(*Serializer1,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  // Serializer 2
  details = SchemaMap.get(1, 2);
  MockHistogramSerializer *Serializer2 =
      dynamic_cast<MockHistogramSerializer *>(
        details->GetSerializer<SchemaType::DA00>());
  expectedTime = 2 * ESSTime::ESSClockTick;
  expectedData = 500;
  EXPECT_CALL(*Serializer2,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  // Serializer 3
  details = SchemaMap.get(2, 1);
  MockHistogramSerializer *Serializer3 =
      dynamic_cast<MockHistogramSerializer *>(
        details->GetSerializer<SchemaType::DA00>());
  expectedTime = 3 * ESSTime::ESSClockTick;
  expectedData = 16777215;
  EXPECT_CALL(*Serializer3,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  // =========================================================================
  // Initialize Test Data
  // =========================================================================
  makeHeader(ESSHeaderParser->Packet, ValidIBMReadouts);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  // check parser counters are as expected
  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 4);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorType, 0);

  // check monitor readouts are processed correctly
  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.CbmCounts, 4);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 4);
  EXPECT_EQ(CbmCounters.IBMEvents, 4);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);
  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_TOF_COUNT),
            4);
}

///
/// \brief Test IBM readout normalization with NADC = FFFFFF MCASum = 8
///
/// Test method is focused only on normalize ADC value.
TEST_F(CbmInstrumentTest, TestIBMNormalizeReadouts) {

  // =========================================================================
  // Test Data - NADC and MCASum value
  // =========================================================================
  std::vector<uint8_t> Readouts{
      0x16, 0x02, 0x14, 0x00, // Fiber 22, FEN 2, Data Length 20
      0x01, 0x00, 0x00, 0x00, // Time HI 1 s
      0xA1, 0x86, 0x01, 0x00, // Time LO 100001 tick
      0x03, 0x01, 0x01, 0x00, // Type 3, Ch 1, ADC 1
      0xFF, 0xFF, 0xFF, 0x08  // NADC (3 bytes), MCA Sum (8)
  };

  // =========================================================================
  // Set Mock Expectations
  // =========================================================================
  int FenId = 2;
  int ChannelId = 1;
  // Serializer 
  SchemaDetails *details = SchemaMap.get(FenId, ChannelId);
  MockHistogramSerializer *Serializer =
      dynamic_cast<MockHistogramSerializer *>(
        details->GetSerializer<SchemaType::DA00>());
  int expectedTime = 1 * ESSTime::ESSClockTick;
  int expectedNorm = 8;
  int expectedData = 0xFFFFFF / expectedNorm;
  EXPECT_CALL(*Serializer,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  makeHeader(ESSHeaderParser->Packet, Readouts);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  // check parser counters are as expected
  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 1);
  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.IBMEvents, 1);
}

///
/// \brief Test IBM readout where EFU configuration has disabled normalize
/// functionality regardless of what is in the data packet
///
/// Test method is focused only on normalize ADC value.
TEST_F(CbmInstrumentTest, TestIBMDisableNormalizeReadouts) {

  // =========================================================================
  // Test Setup - Disable normalization
  // =========================================================================
  this->Configuration->CbmParms.NormalizeIBMReadouts = false;

  // =========================================================================
  // Test Data - NADC and MCASum value
  // =========================================================================
  std::vector<uint8_t> Readouts{
      0x16, 0x01, 0x14, 0x00, // Fiber 22, FEN 1, Data Length 20
      0x01, 0x00, 0x00, 0x00, // Time HI 1 s
      0xA1, 0x86, 0x01, 0x00, // Time LO 100001 tick
      0x03, 0x02, 0x01, 0x00, // Type 3, Ch 2, ADC 1
      0xFF, 0xFF, 0xFF, 0x08  // NADC (3 bytes), MCA Sum (8)
  };

  // =========================================================================
  // Set Mock Expectations
  // =========================================================================
  int FenId = 1;
  int ChannelId = 2;
  // Serializer 
  SchemaDetails *details = SchemaMap.get(FenId, ChannelId);
  MockHistogramSerializer *Serializer =
      dynamic_cast<MockHistogramSerializer *>(
        details->GetSerializer<SchemaType::DA00>());
  int expectedTime = 1 * ESSTime::ESSClockTick;
  int expectedData = 0xFFFFFF;
  EXPECT_CALL(*Serializer,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(AtLeast(1));

  makeHeader(ESSHeaderParser->Packet, Readouts);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  // check parser counters are as expected
  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 1);
  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.IBMEvents, 1);

  // =========================================================================
  // Cleanup - Rollback normalize flag changes
  // =========================================================================
  this->Configuration->CbmParms.NormalizeIBMReadouts = true;
}

/// \brief Test case for monitor readout with Type not supported
/// After support of all instruments it will now test for an error. Unknown
/// instruments increment ErrorType counter
TEST_F(CbmInstrumentTest, TypeNotSupportedError) {
  makeHeader(ESSHeaderParser->Packet, NotSupportedTypeReadout);

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 1);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorType, 1);

  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.TypeNotConfigured, 0);
  EXPECT_EQ(CbmCounters.CbmCounts, 0);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);
}

///
/// \brief Test case for missing serializer configuration error.
/// This test verifies that when FEN/Channel combinations are not in the
/// configuration, the NoSerializerCfgError counter is incremented because
/// no geometry exists for the given topology.
///
TEST_F(CbmInstrumentTest, NoSerializerConfigError) {
  makeHeader(ESSHeaderParser->Packet, FenAndChannelNotInCfgReadout);

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 3);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.CbmCounts,
            0); // No valid events due to validation failure
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 3);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents,
            0); // No events due to validation failure
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);

  // Verify validation errors are tracked via Stats
  // All 3 readouts have FEN=6 which is not in configuration (no geometry
  // exists)
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 3);

  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_TOF_COUNT),
            3);
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
  EV44SerializerMock *Serializer =
    dynamic_cast<EV44SerializerMock *>(
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
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);
  
  // Verify that getTOF detected high TOF
  EXPECT_EQ(ESSHeaderParser->Packet.Time.Counters.TofHigh, 1);
  // clang-format off
  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_TOF_COUNT),0);
  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_TOF_HIGH),1);
  // clang-format on
}

///
/// \brief Test case for the scenario when the readout time is between the
/// previous and current pulse time and the case when is before the previous
/// pulse time
///
TEST_F(CbmInstrumentTest, PreviousTofAndNegativePrevTofErrors) {
  // =========================================================================
  // Set Mock Expectations - One event should be created with non-negative time
  // =========================================================================
  SchemaDetails *details = SchemaMap.get(0, 0);
  EV44SerializerMock *Serializer =
    dynamic_cast<EV44SerializerMock *>(
      details->GetSerializer<SchemaType::EV44>());
  // Verify addEvent is called exactly once with non-negative time value
  EXPECT_CALL(*Serializer, 
              addEvent(Ge(0), Ge(0)))
      .Times(1);

  makeHeader(ESSHeaderParser->Packet, PreviousAndNegativePrevTofReadouts);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(2, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 100000));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 2);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.CbmCounts, 1);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 1);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 1);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);
  
  // Verify that getTOF used previous pulse for first readout and detected negative prev TOF for second
  EXPECT_EQ(ESSHeaderParser->Packet.Time.Counters.TofNegative, 2);
  EXPECT_EQ(ESSHeaderParser->Packet.Time.Counters.PrevTofCount, 1);
  EXPECT_EQ(ESSHeaderParser->Packet.Time.Counters.PrevTofNegative, 1);
  // clang-format off
  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_TOF_HIGH),0);
  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_TOF_COUNT),0);
  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_PREVTOF_COUNT),1);
  EXPECT_EQ(Stats->getValueByName(ESSParser::METRIC_EVENTS_TIMESTAMP_PREVTOF_NEGATIVE),1);
  // clang-format on
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
