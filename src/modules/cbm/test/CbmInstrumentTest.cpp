// Copyright (C) 2021 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Unit test for the CbmInstrument class.
//===----------------------------------------------------------------------===//

#include <common/kafka/EV44Serializer.h>
#include <common/kafka/serializer/DA00HistogramSerializer.h>
#include <common/reduction/Event.h>
#include <common/testutils/HeaderFactory.h>
#include <common/testutils/TestBase.h>
#include <gmock/gmock.h>
#include <memory>
#include <modules/cbm/CbmInstrument.h>

using namespace cbm;
using namespace ESSReadout;

// clang-format off


/// \brief Monitor readout with valid TTL readouts with time values
std::vector<uint8_t> ValidTTLReadouts {
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

/// \brief Monitor readout with valid IBM readouts with time and NPOS values
std::vector<uint8_t> ValidIBMReadouts {
  // Test low 8bit NPOS value
  0x16, 0x01, 0x14, 0x00,  // Fiber 22, FEN 1, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
  0x03, 0x01, 0x00, 0x00,  // Type 3, Ch 1, ADC 1
  0x0A, 0x00, 0x00, 0x00,  // NPOS 10

  // Test medium 16bit NPOS value
  0x16, 0x01, 0x14, 0x00,  // Fiber 22, FEN 1, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA2, 0x86, 0x01, 0x00,  // Time LO 100002 tick
  0x03, 0x02, 0x00, 0x00,  // Type 3, Ch 2, ADC 1
  0xF4, 0x01, 0x00, 0x00,  // NPOS 500

  // Test high 32bit NPOS value, should be processed as 24bit
  0x16, 0x02, 0x14, 0x00,  // Fiber 22, FEN 2, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0xA3, 0x86, 0x01, 0x00,  // Time LO 100003 tick
  0x03, 0x01, 0x00, 0x00,  // Type 3, Ch 0, ADC 1
  0xFF, 0xFF, 0xFF, 0xFF   // XPos 0, YPos 0
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
std::vector<uint8_t> NotSuppoertedTypeReadout {
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x11, 0x00, 0x00, 0x00,  // Time LO 17 ticks
  0x06, 0x00, 0x00, 0x00,  // Type 6, Ch 0, ADC 0
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

/// \brief Monitor readout with TOF value higer then MaxTof limit
std::vector<uint8_t> TofToHighReadout {
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x0A, 0x00, 0x00, 0x00,  // Time HI 10 s
  0xA1, 0x86, 0x01, 0x00,  // Time LO 100001 tick
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

/// \brief Monitor readout to test all negative TOF scenarios
std::vector<uint8_t> PreviousAndNegativPrevTofReadouts {
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
// clang-format on

using namespace fbserializer;

class MockEV44Serializer : public EV44Serializer {
public:
  MOCK_METHOD(size_t, addEvent, (int32_t time, int32_t data), (override));

  MockEV44Serializer() : EV44Serializer(0, "cbm") {}
};

class MockHistogramSerializer : public HistogramSerializer<int32_t> {
public:
  MOCK_METHOD(void, addEvent, (int32_t time, int32_t data), (override));

  MockHistogramSerializer()
      : HistogramSerializer("cbm", 10, 10, "serializer", "A", "ns",
                            BinningStrategy::LastBin) {}
};

class CbmInstrumentTest : public TestBase {
public:
protected:
  struct Counters counters;
  BaseSettings Settings;
  Config Configuration;
  HashMap2D<EV44Serializer> EV44SerializerPtrs{11};
  HashMap2D<HistogramSerializer<int32_t>> HistogramSerializerPtrs{11};
  CbmInstrument *cbm;
  std::unique_ptr<TestHeaderFactory> headerFactory;
  Event TestEvent;           // used for testing generateEvents()
  std::vector<Event> Events; // used for testing generateEvents()

  void SetUp() override {
    Settings.ConfigFile = CBM_CONFIG;

    Configuration = Config(Settings.ConfigFile);
    Configuration.loadAndApply();

    initializeSerializers();

    counters = {};

    headerFactory = std::make_unique<TestHeaderFactory>();
    cbm = new CbmInstrument(counters, Configuration, EV44SerializerPtrs,
                            HistogramSerializerPtrs);
    cbm->ESSHeaderParser.Packet.HeaderPtr =
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

private:
  void initializeSerializers() {
    for (auto &Topology : Configuration.TopologyMapPtr->toValuesList()) {
      if (Topology->Type == CbmType::TTL) {
        std::unique_ptr<EV44Serializer> SerializerPtr =
            std::make_unique<MockEV44Serializer>();
        EV44SerializerPtrs.add(Topology->FEN, Topology->Channel, SerializerPtr);
      } else if (Topology->Type == CbmType::IBM) {
        std::unique_ptr<HistogramSerializer<int32_t>> SerializerPtr =
            std::make_unique<MockHistogramSerializer>();
        HistogramSerializerPtrs.add(Topology->FEN, Topology->Channel,
                                    SerializerPtr);
      }
    }
  }
};

// Test cases below
TEST_F(CbmInstrumentTest, Constructor) { ASSERT_EQ(counters.RingCfgError, 0); }

///
/// \brief Test case for validating TTL type readouts.
///
/// This test case sets expectations on the mocked serializer objects and checks
/// if the monitor readouts are processed correctly. It verifies the parser
/// counters and the processed readout counts. Also test that the serializer's
/// addEvent is called with proper arguments.
///
TEST_F(CbmInstrumentTest, TestValidTTLTypeReadouts) {

  // Set expectations on the mocked serializer objects, one for each monitor
  // readout
  // Serializer 1
  int expectedTime = 1 * ESSTime::ESSClockTick;
  int expectedData = 0;
  MockEV44Serializer *Serializer1 =
      dynamic_cast<MockEV44Serializer *>(EV44SerializerPtrs.get(0, 0));
  EXPECT_CALL(*Serializer1,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(1));

  expectedTime = 2 * ESSTime::ESSClockTick;
  expectedData = 1;
  MockEV44Serializer *Serializer2 =
      dynamic_cast<MockEV44Serializer *>(EV44SerializerPtrs.get(0, 1));
  EXPECT_CALL(*Serializer2,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(1));

  expectedTime = 3 * ESSTime::ESSClockTick;
  expectedData = 3;
  MockEV44Serializer *Serializer3 =
      dynamic_cast<MockEV44Serializer *>(EV44SerializerPtrs.get(1, 0));
  EXPECT_CALL(*Serializer3,
              addEvent(testing::Eq(expectedTime), testing::Eq(expectedData)))
      .Times(testing::AtLeast(1));

  // initialze test data
  makeHeader(cbm->ESSHeaderParser.Packet, ValidTTLReadouts);
  cbm->ESSHeaderParser.Packet.Time.setReference(ESSTime(1, 100000));
  cbm->ESSHeaderParser.Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(cbm->ESSHeaderParser.Packet);
  counters.CbmStats = cbm->CbmReadoutParser.Stats;

  // check parser counters are as expected
  EXPECT_EQ(counters.CbmStats.Readouts, 3);
  EXPECT_EQ(counters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(counters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(counters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(counters.CbmStats.ErrorType, 0);

  // check monitor readouts are processed correctly
  cbm->processMonitorReadouts();
  EXPECT_EQ(counters.RingCfgError, 0);
  EXPECT_EQ(counters.CbmCounts, 3);
  EXPECT_EQ(counters.NoSerializerCfgError, 0);
  EXPECT_EQ(counters.TTLReadoutsProcessed, 3);
  EXPECT_EQ(counters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(counters.IBMEvents, 0);
  EXPECT_EQ(counters.TTLEvents, 3);
  EXPECT_EQ(counters.TimeStats.TofCount, 3);
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

  // initialze test data
  makeHeader(cbm->ESSHeaderParser.Packet, ValidIBMReadouts);
  cbm->ESSHeaderParser.Packet.Time.setReference(ESSTime(1, 100000));
  cbm->ESSHeaderParser.Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(cbm->ESSHeaderParser.Packet);
  counters.CbmStats = cbm->CbmReadoutParser.Stats;

  // check parser counters are as expected
  EXPECT_EQ(counters.CbmStats.Readouts, 3);
  EXPECT_EQ(counters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(counters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(counters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(counters.CbmStats.ErrorType, 0);

  // check monitor readouts are processed correctly
  cbm->processMonitorReadouts();
  EXPECT_EQ(counters.RingCfgError, 0);
  EXPECT_EQ(counters.CbmCounts, 3);
  EXPECT_EQ(counters.NoSerializerCfgError, 0);
  EXPECT_EQ(counters.TTLReadoutsProcessed, 0);
  EXPECT_EQ(counters.IBMReadoutsProcessed, 3);
  EXPECT_EQ(counters.IBMEvents, 3);
  EXPECT_EQ(counters.TTLEvents, 0);
  EXPECT_EQ(counters.TimeStats.TofCount, 3);
}

///
/// \brief Test fixture for the RingConfigurationError test case.
///
/// This test case verifies the behavior of the system when a ring configuration
/// error occurs. It sets up the necessary conditions, triggers the error, and
/// checks the expected counters and statistics.
///
TEST_F(CbmInstrumentTest, RingConfigurationError) {
  makeHeader(cbm->ESSHeaderParser.Packet, RingNotInCfgReadout);

  cbm->CbmReadoutParser.parse(cbm->ESSHeaderParser.Packet);
  counters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(counters.CbmStats.Readouts, 1);
  EXPECT_EQ(counters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(counters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(counters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(counters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(counters.RingCfgError, 1);
  EXPECT_EQ(counters.CbmCounts, 0);
  EXPECT_EQ(counters.NoSerializerCfgError, 0);
  EXPECT_EQ(counters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(counters.TTLReadoutsProcessed, 0);
  EXPECT_EQ(counters.TimeStats.TofCount, 0);
}

/// \brief Test case for monitor readout with Type not supported
/// \note This test is temorary because since all CBM are supported metric will
/// be removed
TEST_F(CbmInstrumentTest, TypeNotSupportedError) {
  makeHeader(cbm->ESSHeaderParser.Packet, NotSuppoertedTypeReadout);

  cbm->CbmReadoutParser.parse(cbm->ESSHeaderParser.Packet);
  counters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(counters.CbmStats.Readouts, 1);
  EXPECT_EQ(counters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(counters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(counters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(counters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(counters.RingCfgError, 0);
  EXPECT_EQ(counters.TypeNotSupported, 1);
  EXPECT_EQ(counters.CbmCounts, 0);
  EXPECT_EQ(counters.NoSerializerCfgError, 0);
  EXPECT_EQ(counters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(counters.TTLReadoutsProcessed, 0);
  EXPECT_EQ(counters.IBMEvents, 0);
  EXPECT_EQ(counters.TTLEvents, 0);
}

///
/// \brief Test case for the scenario when there is no serializer defined for a
/// certain readout. This test verifies the behavior of the CbmInstrument class
/// when the readout arrives for a monitor which is not configured for the EFU.
///
TEST_F(CbmInstrumentTest, NoSerializerCfgError) {
  makeHeader(cbm->ESSHeaderParser.Packet, FenAndChannelNotInCfgReadout);

  cbm->CbmReadoutParser.parse(cbm->ESSHeaderParser.Packet);
  counters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(counters.CbmStats.Readouts, 3);
  EXPECT_EQ(counters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(counters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(counters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(counters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(counters.RingCfgError, 0);
  EXPECT_EQ(counters.CbmCounts, 0);
  EXPECT_EQ(counters.NoSerializerCfgError, 3);
  EXPECT_EQ(counters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(counters.TTLReadoutsProcessed, 3);
  EXPECT_EQ(counters.IBMEvents, 0);
  EXPECT_EQ(counters.TTLEvents, 0);
  EXPECT_EQ(counters.TimeStats.TofCount, 3);
}

///
/// \brief Test case for the scenario when the calculated TOF is higher then the
/// MaxTof limit configured in the configuration file
///
TEST_F(CbmInstrumentTest, TOFHighError) {
  makeHeader(cbm->ESSHeaderParser.Packet, TofToHighReadout);
  cbm->ESSHeaderParser.Packet.Time.setReference(ESSTime(1, 100000));
  cbm->ESSHeaderParser.Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmReadoutParser.parse(cbm->ESSHeaderParser.Packet);
  counters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(counters.CbmStats.Readouts, 1);
  EXPECT_EQ(counters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(counters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(counters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(counters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(counters.RingCfgError, 0);
  EXPECT_EQ(counters.CbmCounts, 0);
  EXPECT_EQ(counters.NoSerializerCfgError, 0);
  EXPECT_EQ(counters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(counters.TTLReadoutsProcessed, 0);
  EXPECT_EQ(counters.IBMEvents, 0);
  EXPECT_EQ(counters.TTLEvents, 0);
  EXPECT_EQ(counters.TimeError, 1);
  EXPECT_EQ(counters.TimeStats.TofCount, 0);
  EXPECT_EQ(counters.TimeStats.TofHigh, 1);
}

///
/// \brief Test case for the scenario when the readout time is between the
/// previoius and current pulse time and the case when is before the previous
/// pulse time
///
TEST_F(CbmInstrumentTest, PreviousTofAndNegativePrevTofErrors) {
  makeHeader(cbm->ESSHeaderParser.Packet, PreviousAndNegativPrevTofReadouts);
  cbm->ESSHeaderParser.Packet.Time.setReference(ESSTime(2, 100000));
  cbm->ESSHeaderParser.Packet.Time.setPrevReference(ESSTime(1, 100000));

  cbm->CbmReadoutParser.parse(cbm->ESSHeaderParser.Packet);
  counters.CbmStats = cbm->CbmReadoutParser.Stats;

  EXPECT_EQ(counters.CbmStats.Readouts, 2);
  EXPECT_EQ(counters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(counters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(counters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(counters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(counters.RingCfgError, 0);
  EXPECT_EQ(counters.CbmCounts, 1);
  EXPECT_EQ(counters.NoSerializerCfgError, 0);
  EXPECT_EQ(counters.IBMReadoutsProcessed, 0);
  EXPECT_EQ(counters.TTLReadoutsProcessed, 1);
  EXPECT_EQ(counters.IBMEvents, 0);
  EXPECT_EQ(counters.TTLEvents, 1);
  EXPECT_EQ(counters.TimeError, 1);
  EXPECT_EQ(counters.TimeStats.TofHigh, 0);
  EXPECT_EQ(counters.TimeStats.TofCount, 0);
  EXPECT_EQ(counters.TimeStats.PrevTofCount, 1);
  EXPECT_EQ(counters.TimeStats.PrevTofNegative, 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
