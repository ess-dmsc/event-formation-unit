// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Unit test for the CbmInstrument class.
//===----------------------------------------------------------------------===//

#include <common/readout/ess/Parser.h>
#include <common/testutils/HeaderFactory.h>
#include <common/testutils/TestBase.h>
#include <modules/cbm/CbmInstrument.h>

using namespace cbm;
using namespace ESSReadout;

using std::filesystem::path;

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
std::vector<uint8_t> InvalidEvent2DReadouts {
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
  "NormalizeIBMReadouts": true,

  "Topology" : [
    { "FEN": 0, "Channel": 0, "Type": "EVENT_0D", "Source" : "cbm1", "PixelOffset": 0 },
    { "FEN": 0, "Channel": 1, "Type": "EVENT_0D", "Source" : "cbm2", "PixelOffset": 1 },
    { "FEN": 1, "Channel": 0, "Type": "EVENT_0D", "Source" : "cbm3", "PixelOffset": 3 },
    { "FEN": 1, "Channel": 1, "Type": "IBM", "Source" : "cbm4", "MaxTofBin": 10000, "BinCount": 100 },
    { "FEN": 1, "Channel": 2, "Type": "IBM", "Source" : "cbm5", "MaxTofBin": 10000, "BinCount": 100 },
    { "FEN": 2, "Channel": 1, "Type": "IBM", "Source" : "cbm6", "MaxTofBin": 10000, "BinCount": 100 },
    { "FEN":  3, "Channel": 0, "Type": "EVENT_2D", "Source" : "cbm7", "Width": 512, "Height": 512 },
    { "FEN":  3, "Channel": 1, "Type": "EVENT_2D", "Source" : "cbm8", "Width": 512, "Height": 512 },
    { "FEN":  3, "Channel": 2, "Type": "EVENT_2D", "Source" : "cbm9", "Width": 512, "Height": 512 }
  ]
}
)"_json;

// clang-format on

using namespace fbserializer;

class Mock0DimEV44Serializer : public EV44Serializer {
public:
  MOCK_METHOD(size_t, addEvent, (int32_t time, int32_t data), (override));

  Mock0DimEV44Serializer() : EV44Serializer(0, "cbm") {}
};

class Mock2DimEV44Serializer : public EV44Serializer {
public:
  MOCK_METHOD(size_t, addEvent, (int32_t time, int32_t data), (override));

  Mock2DimEV44Serializer() : EV44Serializer(0, "cbm") {}
};


class MockHistogramSerializer : public HistogramSerializer<int32_t> {
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
  std::unique_ptr<Config> Configuration; // Changed to unique_ptr
  TestHeaderFactory headerFactory;

  /// Objects required for the CbmInstrument
  /// \note These are initialized in SetUp()
  std::unique_ptr<Statistics> Stats;
  std::unique_ptr<ESSReadout::Parser> ESSHeaderParser;
  HashMap2D<EV44Serializer> EV44SerializerPtrs{11};
  HashMap2D<HistogramSerializer<int32_t>> HistogramSerializerPtrs{11};
  std::unique_ptr<CbmInstrument> cbm;

  inline static path FullConfigFile{""};

  void SetUp() override {
    // Initialize stats and parser
    Stats = std::make_unique<Statistics>();
    ESSHeaderParser = std::make_unique<ESSReadout::Parser>(*Stats);

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
      if (Topology->Type == CbmType::EVENT_0D) {
        std::unique_ptr<EV44Serializer> SerializerPtr =
            std::make_unique<Mock0DimEV44Serializer>();
        EV44SerializerPtrs.add(Topology->FEN, Topology->Channel, SerializerPtr);
      } else if (Topology->Type == CbmType::EVENT_2D) {
        std::unique_ptr<EV44Serializer> SerializerPtr =
            std::make_unique<Mock2DimEV44Serializer>();
        EV44SerializerPtrs.add(Topology->FEN, Topology->Channel, SerializerPtr);
      } else if (Topology->Type == CbmType::IBM) {
        std::unique_ptr<HistogramSerializer<int32_t>> SerializerPtr =
            std::make_unique<MockHistogramSerializer>();
        HistogramSerializerPtrs.add(Topology->FEN, Topology->Channel,
                                    SerializerPtr);
      }
    }
  }

  void initializeCbmInstrument() {
    cbm = std::make_unique<CbmInstrument>(
        CbmCounters, *Configuration, EV44SerializerPtrs,
        HistogramSerializerPtrs, *ESSHeaderParser);
  }
};

// Test cases below
TEST_F(CbmInstrumentTest, Constructor) {
  ASSERT_EQ(CbmCounters.RingCfgError, 0);
}

///
/// \brief Test case for validating Event0D type readouts.
///
/// This test case sets expectations on the mocked serializer objects and checks
/// if the monitor readouts are processed correctly. It verifies the parser
/// counters and the processed readout counts. Also test that the serializer's
/// addEvent is called with proper arguments.
///
TEST_F(CbmInstrumentTest, TestValidEvent0DTypeReadouts) {

  // Set expectations on the mocked serializer objects, one for each monitor
  // readout
  // Serializer 1
  int expectedTime = 1 * ESSTime::ESSClockTick;
  int expectedData = 0;
  Mock0DimEV44Serializer *Serializer1 =
      dynamic_cast<Mock0DimEV44Serializer *>(EV44SerializerPtrs.get(0, 0));
  EXPECT_CALL(*Serializer1,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(1));

  expectedTime = 2 * ESSTime::ESSClockTick;
  expectedData = 1;
  Mock0DimEV44Serializer *Serializer2 =
      dynamic_cast<Mock0DimEV44Serializer *>(EV44SerializerPtrs.get(0, 1));
  EXPECT_CALL(*Serializer2,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(1));

  expectedTime = 3 * ESSTime::ESSClockTick;
  expectedData = 3;
  Mock0DimEV44Serializer *Serializer3 =
      dynamic_cast<Mock0DimEV44Serializer *>(EV44SerializerPtrs.get(1, 0));
  EXPECT_CALL(*Serializer3,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(1));

  // initialze test data
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
  EXPECT_EQ(CbmCounters.RingCfgError, 0);
  EXPECT_EQ(CbmCounters.CbmCounts, 3);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 3);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 3);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);
  EXPECT_EQ(
      Stats->getValueByName(ESSHeaderParser->METRIC_EVENTS_TIMESTAMP_TOF_COUNT), 3);
}


///
/// \brief Test case for validating Event2D type readouts using valid data.
///
/// This test case sets expectations on the mocked serializer objects and checks
/// if the monitor readouts are processed correctly. It verifies the parser
/// counters and the processed readout counts. Also test that the serializer's
/// addEvent is called with proper arguments.
///
TEST_F(CbmInstrumentTest, TestValidEvent2DTypeReadouts) {

  const int HorizontalWidth = 512;
  // Set expectations on the mocked serializer objects, one for each monitor
  // readout
  // Serializer 1. Test pixel id calculation
  // Serializer 1
  int expectedTime = 1 * ESSTime::ESSClockTick;
  // Test Data => XPos 256, YPos 257
  int expectedData = 257 * HorizontalWidth + 256 + 1;
  Mock2DimEV44Serializer *Serializer1 =
      dynamic_cast<Mock2DimEV44Serializer *>(EV44SerializerPtrs.get(3, 0));
  EXPECT_CALL(*Serializer1,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(1));

  expectedTime = 2 * ESSTime::ESSClockTick;
  // Test Data => XPos 426, YPos 443
  expectedData = 443 * HorizontalWidth + 426 + 1;
  Mock2DimEV44Serializer *Serializer2 =
      dynamic_cast<Mock2DimEV44Serializer *>(EV44SerializerPtrs.get(3, 1));
  EXPECT_CALL(*Serializer2,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(1));

  expectedTime = 3 * ESSTime::ESSClockTick;
  // Test Data => XPos 204, YPos 221
  expectedData = 221 * HorizontalWidth + 204 + 1;
  Mock2DimEV44Serializer *Serializer3 =
      dynamic_cast<Mock2DimEV44Serializer *>(EV44SerializerPtrs.get(3, 2));
  EXPECT_CALL(*Serializer3,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(1));

  // initialize test data
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

  // check monitor readouts are processed correctly
  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.RingCfgError, 0);
  EXPECT_EQ(CbmCounters.CbmCounts, 3);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 3);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 3);
  EXPECT_EQ(
      Stats->getValueByName(ESSHeaderParser->METRIC_EVENTS_TIMESTAMP_TOF_COUNT), 3);
}


///
/// \brief Test case for validating Event2D type readouts using invalid data.
///
/// This test case sets expectations on the mocked serializer objects and checks
/// if the monitor readouts are processed correctly. It verifies the parser
/// counters and the processed readout counts. Also test that the serializer's
/// addEvent is called with proper arguments.
///
TEST_F(CbmInstrumentTest, TestInvalidEvent2DTypeReadouts) {

  // Set expectations on the mocked serializer objects, one for each monitor
  // readout
  int expectedTime = 0;
  int expectedData = 0;
  Mock2DimEV44Serializer *Serializer1 =
      dynamic_cast<Mock2DimEV44Serializer *>(EV44SerializerPtrs.get(3, 0));
  EXPECT_CALL(*Serializer1,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(0));

  Mock2DimEV44Serializer *Serializer2 =
      dynamic_cast<Mock2DimEV44Serializer *>(EV44SerializerPtrs.get(3, 1));
  EXPECT_CALL(*Serializer2,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(0));

  Mock2DimEV44Serializer *Serializer3 =
      dynamic_cast<Mock2DimEV44Serializer *>(EV44SerializerPtrs.get(3, 2));
  EXPECT_CALL(*Serializer3,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(0));

  // initialize test data
  makeHeader(ESSHeaderParser->Packet, InvalidEvent2DReadouts);
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
  EXPECT_EQ(CbmCounters.RingCfgError, 0);
  EXPECT_EQ(CbmCounters.CbmCounts, 3);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 3);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 3);
  EXPECT_EQ(cbm->CbmReadoutParser.Stats.ErrorADC, 3);
  EXPECT_EQ(
      Stats->getValueByName(ESSHeaderParser->METRIC_EVENTS_TIMESTAMP_TOF_COUNT), 3);
}

/// \brief Test case for validating IBM type readouts.
///
/// This test case sets expectations on the mocked serializer objects and checks
/// if the monitor readouts are processed correctly. It verifies the parser
/// counters and the processed readout counts. Also test that the serializer's
/// addEvent is called with proper arguments.
///
TEST_F(CbmInstrumentTest, TestValidIBMTypeReadouts) {

  // Set expectations on the mocked serializer objects, one for each monitor
  // readout
  // Serializer 1
  MockHistogramSerializer *Serializer1 =
      dynamic_cast<MockHistogramSerializer *>(
          HistogramSerializerPtrs.get(1, 1));
  int expectedTime = 1 * ESSTime::ESSClockTick;
  int expectedData = 10;
  EXPECT_CALL(*Serializer1,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(1));

  // Serializer 2
  MockHistogramSerializer *Serializer2 =
      dynamic_cast<MockHistogramSerializer *>(
          HistogramSerializerPtrs.get(1, 2));
  expectedTime = 2 * ESSTime::ESSClockTick;
  expectedData = 500;
  EXPECT_CALL(*Serializer2,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(1));

  // Serializer 3
  MockHistogramSerializer *Serializer3 =
      dynamic_cast<MockHistogramSerializer *>(
          HistogramSerializerPtrs.get(2, 1));
  expectedTime = 3 * ESSTime::ESSClockTick;
  expectedData = 16777215;
  EXPECT_CALL(*Serializer3,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(1));

  // initialize test data
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
  EXPECT_EQ(CbmCounters.RingCfgError, 0);
  EXPECT_EQ(CbmCounters.CbmCounts, 4);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 4);
  EXPECT_EQ(CbmCounters.IBMEvents, 4);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);
  EXPECT_EQ(
      Stats->getValueByName(ESSHeaderParser->METRIC_EVENTS_TIMESTAMP_TOF_COUNT),
      4);
}

///
/// \brief Test IBM readout normalization with NADC = FFFFFF MCASum = 8
///
/// Test method is focused only on normalize ADC value.
TEST_F(CbmInstrumentTest, TestIBMNormalizeReadouts) {
    std::vector<uint8_t> Readouts {
    // Test NDAC and MCASum value
    0x16, 0x02, 0x14, 0x00,  // Fiber 22, FEN 2, Data Length 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
    0x03, 0x01, 0x01, 0x00,  // Type 3, Ch 1, ADC 1
    0xFF, 0xFF, 0xFF, 0x08   // NADC (3 bytes), MCA Sum (8)
  };

  int FenId = 2;
  int ChannelId = 1;
  // Serializer 
  MockHistogramSerializer *Serializer =
      dynamic_cast<MockHistogramSerializer *>(
          HistogramSerializerPtrs.get(FenId, ChannelId));
  int expectedTime = 1 * ESSTime::ESSClockTick;
  int expectedNorm = 8;
  int expectedData = 0xFFFFFF / expectedNorm;
  EXPECT_CALL(*Serializer,
     addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(1));

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
  //Update config with new normalize flag
  this->Configuration->CbmParms.NormalizeIBMReadouts = false;
  std::vector<uint8_t> Readouts {
    // Test NDAC and MCASum value
    0x16, 0x01, 0x14, 0x00,  // Fiber 22, FEN 1, Data Length 20
    0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
    0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
    0x03, 0x02, 0x01, 0x00,  // Type 3, Ch 2, ADC 1
    0xFF, 0xFF, 0xFF, 0x08   // NADC (3 bytes), MCA Sum (8)
  };

  int FenId = 1;
  int ChannelId = 2;
  // Serializer 
  MockHistogramSerializer *Serializer =
      dynamic_cast<MockHistogramSerializer *>(
          HistogramSerializerPtrs.get(FenId, ChannelId));
  int expectedTime = 1 * ESSTime::ESSClockTick;
  int expectedData = 0xFFFFFF;
  EXPECT_CALL(*Serializer,
     addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(1));

  makeHeader(ESSHeaderParser->Packet, Readouts);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  // check parser counters are as expected
  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 1);
  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.IBMEvents, 1);
  //Rollback normalize flag changes
  this->Configuration->CbmParms.NormalizeIBMReadouts = true;
}

///
/// \brief Test fixture for the RingConfigurationError test case.
///
/// This test case verifies the behavior of the system when a ring configuration
/// error occurs. It sets up the necessary conditions, triggers the error, and
/// checks the expected counters and statistics.
///
TEST_F(CbmInstrumentTest, RingConfigurationError) {
  makeHeader(ESSHeaderParser->Packet, RingNotInCfgReadout);

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 1);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.RingCfgError, 1);
  EXPECT_EQ(CbmCounters.CbmCounts, 0);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(
      Stats->getValueByName(ESSHeaderParser->METRIC_EVENTS_TIMESTAMP_TOF_COUNT),
      0);
}

/// \brief Test case for monitor readout with Type not supported
/// After support of all instruments it will now test for an error. Unknown instruments increment
/// ErrorType counter
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
  EXPECT_EQ(CbmCounters.RingCfgError, 0);
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
/// \brief Test case for the scenario when there is no serializer defined for a
/// certain readout. This test verifies the behavior of the CbmInstrument class
/// when the readout arrives for a monitor which is not configured for the EFU.
///
TEST_F(CbmInstrumentTest, NoSerializerCfgError) {
  makeHeader(ESSHeaderParser->Packet, FenAndChannelNotInCfgReadout);

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(CbmCounters.CbmStats.Readouts, 3);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(CbmCounters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(CbmCounters.RingCfgError, 0);
  EXPECT_EQ(CbmCounters.CbmCounts, 0);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 3);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 3);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);
  EXPECT_EQ(
      Stats->getValueByName(ESSHeaderParser->METRIC_EVENTS_TIMESTAMP_TOF_COUNT),
      3);
}

///
/// \brief Test case for the scenario when the calculated TOF is higher then the
/// MaxTof limit configured in the configuration file
///
TEST_F(CbmInstrumentTest, HighTofErrorDefaultValue) {
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

  EXPECT_EQ(CbmCounters.RingCfgError, 0);
  EXPECT_EQ(CbmCounters.CbmCounts, 0);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 0);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);
  EXPECT_EQ(CbmCounters.TimeError, 1);
  EXPECT_EQ(
      Stats->getValueByName(ESSHeaderParser->METRIC_EVENTS_TIMESTAMP_TOF_COUNT),
      0);
  EXPECT_EQ(
      Stats->getValueByName(ESSHeaderParser->METRIC_EVENTS_TIMESTAMP_TOF_HIGH),
      1);
}

///
/// \brief Test case for the scenario when MaxTOFNS is set to 0 in the
/// configuration. Any valid readout, even with TOF close to ESS Time, should
/// produce a HighTof error.
///
TEST_F(CbmInstrumentTest, HighTofErrorMaxTofSetInJson) {
  // Modify configuration to set MaxTOFNS to 0
  auto ZeroMaxTofConfig = TestConfig;
  ZeroMaxTofConfig["MaxTOFNS"] = 0;
  Configuration->setRoot(ZeroMaxTofConfig);
  Configuration->apply();

  initializeSerializers();
  initializeCbmInstrument();

  // Use a valid Event0D readout with TOF close to ESS Time
  makeHeader(ESSHeaderParser->Packet, ValidEvent0DReadouts);
  ESSHeaderParser->Packet.Time.setReference(ESSTime(1, 100000));
  ESSHeaderParser->Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(ESSHeaderParser->Packet);
  CbmCounters.CbmStats = cbm->CbmReadoutParser.Stats;

  cbm->processMonitorReadouts();

  // All readouts should be rejected due to MaxTOFNS == 0
  EXPECT_EQ(CbmCounters.TimeError, 3);
  EXPECT_EQ(
      Stats->getValueByName(ESSHeaderParser->METRIC_EVENTS_TIMESTAMP_TOF_HIGH),
      3);
  EXPECT_EQ(
      Stats->getValueByName(ESSHeaderParser->METRIC_EVENTS_TIMESTAMP_TOF_COUNT),
      0);
  EXPECT_EQ(CbmCounters.CbmCounts, 0);
}

///
/// \brief Test case for the scenario when the readout time is between the
/// previous and current pulse time and the case when is before the previous
/// pulse time
///
TEST_F(CbmInstrumentTest, PreviousTofAndNegativePrevTofErrors) {
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
  EXPECT_EQ(CbmCounters.RingCfgError, 0);
  EXPECT_EQ(CbmCounters.CbmCounts, 1);
  EXPECT_EQ(CbmCounters.NoSerializerCfgError, 0);
  EXPECT_EQ(CbmCounters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.Event0DReadoutsProcessed, 1);
  EXPECT_EQ(CbmCounters.Event2DReadoutsProcessed, 0);
  EXPECT_EQ(CbmCounters.IBMEvents, 0);
  EXPECT_EQ(CbmCounters.Event0DEvents, 1);
  EXPECT_EQ(CbmCounters.Event2DEvents, 0);
  EXPECT_EQ(CbmCounters.TimeError, 1);
  EXPECT_EQ(
      Stats->getValueByName(ESSHeaderParser->METRIC_EVENTS_TIMESTAMP_TOF_HIGH),
      0);
  EXPECT_EQ(
      Stats->getValueByName(ESSHeaderParser->METRIC_EVENTS_TIMESTAMP_TOF_COUNT),
      0);
  EXPECT_EQ(Stats->getValueByName(
                ESSHeaderParser->METRIC_EVENTS_TIMESTAMP_PREVTOF_COUNT),
            1);
  EXPECT_EQ(Stats->getValueByName(
                ESSHeaderParser->METRIC_EVENTS_TIMESTAMP_PREVTOF_NEGATIVE),
            1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
