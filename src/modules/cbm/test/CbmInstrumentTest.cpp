// Copyright (C) 2021 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
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

std::vector<uint8_t> ValidTTLReadouts {
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x01, 0x01, 0x00,  // Type 1, Ch 1, ADC 1
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  0x16, 0x01, 0x14, 0x00,  // Fiber 22, FEN 1, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x01, 0x00,  // Type 1, Ch 0, ADC 1
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};

std::vector<uint8_t> ValidIBMReadouts {
  // Test low 8bit NPOS value
  0x16, 0x01, 0x14, 0x00,  // Fiber 22, FEN 1, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x03, 0x01, 0x00, 0x00,  // Type 3, Ch 1, ADC 1
  0x0A, 0x00, 0x00, 0x00,  // NPOS 10

  // Test medium 16bit NPOS value
  0x16, 0x01, 0x14, 0x00,  // Fiber 22, FEN 1, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x03, 0x02, 0x00, 0x00,  // Type 3, Ch 2, ADC 1
  0xF4, 0x01, 0x00, 0x00,  // NPOS 500

  // Test high 32bit NPOS value, should be processed as 24bit
  0x16, 0x02, 0x14, 0x00,  // Fiber 22, FEN 2, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
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

std::vector<uint8_t> MonitorReadoutTOF {
  // First monitor readout - Negative PrevTOF - possibly unreachable!
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x00, 0x00,  // Type 1, Ch 0, ADC 0
  0x00, 0x00, 0x00, 0x00,  // XPos 0, YPos 0

  // Second monitor readout - Negative TOF, positive PrevTOF
  0x16, 0x00, 0x14, 0x00,  // Fiber 22, FEN 0, Data Length 20
  0x01, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x01, 0x00, 0x00, 0x00,  // Type 1, Ch 0, ADC 0
  0x00, 0x00, 0x00, 0x00   // XPos 0, YPos 0
};
// clang-format on

using namespace fbserializer;

class MockEV44Serializer : public EV44Serializer {
public:
  MOCK_METHOD(size_t, addEvent, (int32_t time, int32_t data), (override));

  MockEV44Serializer() : EV44Serializer(0, "cbm") {}
};

class MockHistogramSerializer
    : public HistogramSerializer<int32_t> {
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
  HashMap2D<HistogramSerializer<int32_t>> HistogramSerializerPtrs{
      11};
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
    cbm->ESSReadoutParser.Packet.HeaderPtr =
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
        std::unique_ptr<HistogramSerializer<int32_t>>
            SerializerPtr = std::make_unique<MockHistogramSerializer>();
        HistogramSerializerPtrs.add(Topology->FEN, Topology->Channel,
                                    SerializerPtr);
      }
    }
  }
};

// Test cases below
TEST_F(CbmInstrumentTest, Constructor) { ASSERT_EQ(counters.RingCfgError, 0); }

TEST_F(CbmInstrumentTest, TestValidTTLTypeReadouts) {

  // Set expectations on the mocked serializer objects, one for each monitor
  // readout
  // Serializer 1
  MockEV44Serializer *Serializer1 =
      dynamic_cast<MockEV44Serializer *>(EV44SerializerPtrs.get(0, 0));
  EXPECT_CALL(*Serializer1, addEvent(testing::Eq(0), testing::Eq(0)))
      .Times(testing::AtLeast(1));

  // Serializer 2
  MockEV44Serializer *Serializer2 =
      dynamic_cast<MockEV44Serializer *>(EV44SerializerPtrs.get(0, 1));
  EXPECT_CALL(*Serializer2, addEvent(testing::Eq(0), testing::Eq(1)))
      .Times(testing::AtLeast(1));
  EXPECT_CALL(*Serializer2, addEvent(testing::Eq(0), testing::Eq(2)))
      .Times(testing::AtLeast(1));

  // Serializer 3
  MockEV44Serializer *Serializer3 =
      dynamic_cast<MockEV44Serializer *>(EV44SerializerPtrs.get(1, 0));
  EXPECT_CALL(*Serializer3, addEvent(testing::Eq(0), testing::Eq(3)))
      .Times(testing::AtLeast(1));

  // initialze test data
  makeHeader(cbm->ESSReadoutParser.Packet, ValidTTLReadouts);

  cbm->CbmParser.parse(cbm->ESSReadoutParser.Packet);
  counters.CbmStats = cbm->CbmParser.Stats;

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
  EXPECT_EQ(counters.TTLReadouts, 3);
  EXPECT_EQ(counters.IBMReadouts, 0);
}

TEST_F(CbmInstrumentTest, TestValidIBMTypeReadouts) {

  // Set expectations on the mocked serializer objects, one for each monitor
  // readout
  // Serializer 1
  MockHistogramSerializer *Serializer1 =
      dynamic_cast<MockHistogramSerializer *>(HistogramSerializerPtrs.get(1, 1));
  EXPECT_CALL(*Serializer1, addEvent(testing::Eq(0), testing::Eq(10)))
      .Times(testing::AtLeast(1));

  // Serializer 2
  MockHistogramSerializer *Serializer2 =
      dynamic_cast<MockHistogramSerializer *>(HistogramSerializerPtrs.get(1, 2));
  EXPECT_CALL(*Serializer2, addEvent(testing::Eq(0), testing::Eq(500)))
      .Times(testing::AtLeast(1));

  // Serializer 3
  MockHistogramSerializer *Serializer3 =
      dynamic_cast<MockHistogramSerializer *>(HistogramSerializerPtrs.get(2, 1));
  EXPECT_CALL(*Serializer3, addEvent(testing::Eq(0), testing::Eq(16777215)))
      .Times(testing::AtLeast(1));

  // initialze test data
  makeHeader(cbm->ESSReadoutParser.Packet, ValidIBMReadouts);

  cbm->CbmParser.parse(cbm->ESSReadoutParser.Packet);
  counters.CbmStats = cbm->CbmParser.Stats;

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
  EXPECT_EQ(counters.TTLReadouts, 0);
  EXPECT_EQ(counters.IBMReadouts, 3);
}

TEST_F(CbmInstrumentTest, RingConfigurationError) {
  makeHeader(cbm->ESSReadoutParser.Packet, RingNotInCfgReadout);

  cbm->CbmParser.parse(cbm->ESSReadoutParser.Packet);
  counters.CbmStats = cbm->CbmParser.Stats;

  EXPECT_EQ(counters.CbmStats.Readouts, 1);
  EXPECT_EQ(counters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(counters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(counters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(counters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(counters.RingCfgError, 1);
  EXPECT_EQ(counters.CbmCounts, 0);
  EXPECT_EQ(counters.NoSerializerCfgError, 0);
  EXPECT_EQ(counters.IBMReadouts, 0);
  EXPECT_EQ(counters.TTLReadouts, 0);
}

TEST_F(CbmInstrumentTest, NoSerializerCfgError) {
  makeHeader(cbm->ESSReadoutParser.Packet, FenAndChannelNotInCfgReadout);

  cbm->CbmParser.parse(cbm->ESSReadoutParser.Packet);
  counters.CbmStats = cbm->CbmParser.Stats;

  EXPECT_EQ(counters.CbmStats.Readouts, 3);
  EXPECT_EQ(counters.CbmStats.ErrorFiber, 0);
  EXPECT_EQ(counters.CbmStats.ErrorFEN, 0);
  EXPECT_EQ(counters.CbmStats.ErrorADC, 0);
  EXPECT_EQ(counters.CbmStats.ErrorType, 0);

  cbm->processMonitorReadouts();
  EXPECT_EQ(counters.RingCfgError, 0);
  EXPECT_EQ(counters.CbmCounts, 0);
  EXPECT_EQ(counters.NoSerializerCfgError, 3);
  EXPECT_EQ(counters.IBMReadouts, 0);
  EXPECT_EQ(counters.TTLReadouts, 0);
}

TEST_F(CbmInstrumentTest, BeamMonitorTOF) {
  makeHeader(cbm->ESSReadoutParser.Packet, MonitorReadoutTOF);
  cbm->ESSReadoutParser.Packet.Time.setReference(ESSTime(1, 100000));
  cbm->ESSReadoutParser.Packet.Time.setPrevReference(ESSTime(1, 0));

  cbm->CbmParser.parse(cbm->ESSReadoutParser.Packet);
  counters.CbmStats = cbm->CbmParser.Stats;

  cbm->processMonitorReadouts();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
