// Copyright (C) 2019 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for CaenBase
///
//===----------------------------------------------------------------------===//

#include <caen/CaenBase.h>
#include <common/readout/ess/Parser.h>
#include <common/testutils/TestBase.h>
#include <string>

using namespace ESSReadout;

class CaenBaseTest : public ::testing::Test {
public:
  BaseSettings Settings;

  void SetUp() override {
    Settings.DetectorName = "loki";
    Settings.GraphitePrefix = "loki";
    Settings.GraphiteRegion = "test";
    Settings.SocketRxTimeoutUS = 1000;
    Settings.NoHwCheck = true;
    Settings.ConfigFile = LOKI_CONFIG;
    Settings.CalibFile = LOKI_CALIB;
  }

  void TearDown() override {}
};

TEST_F(CaenBaseTest, LokiConstructor) {
  Caen::CaenBase Readout(Settings, DetectorType::LOKI);
  ASSERT_EQ(Readout.Stats.getStatPrefix(1), "loki.test.");
  EXPECT_EQ(Readout.getInputCounters().RxPackets, 0);
}

TEST_F(CaenBaseTest, BifrostConstructor) {
  Settings.ConfigFile = BIFROST_CONFIG;
  Settings.CalibFile = BIFROST_CALIB;
  Settings.DetectorName = "bifrost";
  Settings.GraphitePrefix = "bifrost";
  Caen::CaenBase Readout(Settings, DetectorType::BIFROST);
  Readout.Counters = {};
  ASSERT_EQ(Readout.Stats.getStatPrefix(1), "bifrost.test.");
  EXPECT_EQ(Readout.getInputCounters().RxPackets, 0);
}

TEST_F(CaenBaseTest, CspecConstructor) {
  Settings.ConfigFile = CSPEC_CONFIG;
  Settings.CalibFile = CSPEC_CALIB;
  Settings.DetectorName = "cspec";
  Settings.GraphitePrefix = "cspec";
  Caen::CaenBase Readout(Settings, DetectorType::CSPEC);
  ASSERT_EQ(Readout.Stats.getStatPrefix(1), "cspec.test.");
  Readout.Counters = {};
  EXPECT_EQ(Readout.getInputCounters().RxPackets, 0);
}

TEST_F(CaenBaseTest, MiraclesConstructor) {
  Settings.ConfigFile = MIRACLES_CONFIG;
  Settings.CalibFile = MIRACLES_CALIB;
  Settings.DetectorName = "miracles";
  Settings.GraphitePrefix = "miracles";
  Caen::CaenBase Readout(Settings, DetectorType::MIRACLES);
  ASSERT_EQ(Readout.Stats.getStatPrefix(1), "miracles.test.");
  Readout.Counters = {};
  EXPECT_EQ(Readout.getInputCounters().RxPackets, 0);
}

std::vector<uint8_t> BadTestPacket{0x00, 0x01, 0x02};

// clang-format off
std::vector<uint8_t> TestPacket2{
    // ESS header
    0x00, 0x00,             // pad, v0
    0x45, 0x53, 0x53, 0x30, //  'E' 'S' 'S' 0x30
    0xae, 0x00, 0x00, 0x00, // 0x96 = 150 bytes
    0x11, 0x00, 0x00, 0x00, // Pulse time High (17s)
    0x00, 0x01, 0x00, 0x00, // Pulse time Low (256 clocks)
    0x11, 0x00, 0x00, 0x00, // Prev PT
    0x00, 0x00, 0x00, 0x00, //
    0x01, 0x00, 0x00, 0x00, // Seq number 1

    // Readout 1
    0x00, 0x00, 0x18, 0x00, // fiber 0, fen 0, data size 64 bytes
    0x11, 0x00, 0x00, 0x00, // time high (17s)
    0x01, 0x01, 0x00, 0x00, // time low (257 clocks)
    0x00, 0x00, 0x00, 0x00, // fpga 0, tube 0
    0x01, 0x01, 0x02, 0x01, // amp a, amp b
    0x03, 0x01, 0x04, 0x01, // amp c, amp d

    // Readout 2            Fiber 20 (Ring 10) is invalid -> RingErrors++
    0x14, 0x00, 0x18, 0x00, // fiber 20, fen 0, data size 64 bytes
    0x11, 0x00, 0x00, 0x00, // time high 17s
    0x01, 0x02, 0x00, 0x00, // time low (257 clocks)
    0x00, 0x00, 0x00, 0x00, // fpga 0, tube 0
    0x01, 0x01, 0x02, 0x01, // amp a, amp b
    0x03, 0x01, 0x04, 0x01, // amp c, amp d

    // Readout 3            FEN 19 is invalid -> FENErrors++ (for loki only so far)
    0x01, 0x13, 0x18, 0x00, // fiber 1, fen 19, size 24 bytes
    0x11, 0x00, 0x00, 0x00, // time high (17s)
    0x02, 0x02, 0x00, 0x00, // time low (258 clocks)
    0x00, 0x00, 0x00, 0x00, // fpga 0, tube 0
    0x01, 0x02, 0x02, 0x02, // amp a, amp b
    0x03, 0x02, 0x04, 0x02, // amp c, amp d

    // Readout 4            amplitudes are all 0 -> PixelErrors ++
    0x00, 0x00, 0x18, 0x00, // ring 0, fen 0, data size 64 bytes
    0x11, 0x00, 0x00, 0x00, // time high (17s)
    0x03, 0x01, 0x00, 0x00, // time low (259 clocks)
    0x00, 0x00, 0x00, 0x00, // amplitudes are all 0, PixelErrors ++
    0x00, 0x00, 0x00, 0x00, // fpga 0, tube 0
    0x00, 0x00, 0x00, 0x00, // amp a, amp b

    // Readout 5
    0x00, 0x00, 0x18, 0x00, // ring 0, fen 0, data size 64 bytes
    0x14, 0x00, 0x00, 0x00, // time high (20s)
    0x01, 0x01, 0x00, 0x00, // time low (257 clocks)
    0x00, 0x00, 0x00, 0x00, // fpga 0, tube 0
    0x01, 0x01, 0x02, 0x01, // amp a, amp b
    0x03, 0x01, 0x04, 0x01, // amp c, amp d

    // Readout 6
    0x00, 0x00, 0x18, 0x00, // ring 0, fen 0, data size 64 bytes
    0x0a, 0x00, 0x00, 0x00, // time high (10s)
    0x01, 0x01, 0x00, 0x00, // time low (257 clocks)
    0x00, 0x00, 0x00, 0x00, // fpga 0, tube 0
    0x01, 0x01, 0x02, 0x01, // amp a, amp b
    0x03, 0x01, 0x04, 0x01, // amp c, amp d
};
// clang-format on

TEST_F(CaenBaseTest, DataReceiveLoki) {
  Caen::CaenBase Readout(Settings, DetectorType::LOKI);

  writePacketToRxFIFO(Readout, BadTestPacket);

  EXPECT_EQ(
      Readout.Stats.getValueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_SIZE),
      1);
  EXPECT_EQ(Readout.Counters.Parser.Readouts, 0);
  EXPECT_NE(Readout.getInputCounters().RxIdle, 0);
  EXPECT_NE(Readout.Counters.ProcessingIdle, 0);
  Readout.stopThreads();
}

TEST_F(CaenBaseTest, DataReceiveBifrost) {
  Settings.DetectorName = "bifrost";
  Settings.ConfigFile = BIFROST_CONFIG;
  Settings.CalibFile = BIFROST_CALIB;
  Caen::CaenBase Readout(Settings, DetectorType::BIFROST);

  writePacketToRxFIFO(Readout, TestPacket2);

  EXPECT_EQ(Readout.Stats.getValueByName(
                ESSReadout::Parser::METRIC_PARSER_ESSHEADER_ERRORS_TYPE),
            1);
  EXPECT_EQ(Readout.Counters.Parser.Readouts, 0);
  Readout.stopThreads();
}

TEST_F(CaenBaseTest, DataReceiveMiracles) {
  Settings.DetectorName = "miracles";
  Settings.ConfigFile = MIRACLES_CONFIG;
  Settings.CalibFile = MIRACLES_CALIB;
  Caen::CaenBase Readout(Settings, DetectorType::MIRACLES);

  writePacketToRxFIFO(Readout, TestPacket2);

  EXPECT_EQ(Readout.Counters.Parser.Readouts, 0);
  Readout.stopThreads();
}

TEST_F(CaenBaseTest, DataReceiveGoodLoki) {
  Caen::CaenBase Readout(Settings, DetectorType::LOKI);

  writePacketToRxFIFO(Readout, TestPacket2);

  EXPECT_EQ(Readout.Counters.Parser.Readouts, 6);
  EXPECT_EQ(Readout.Counters.Parser.DataHeaders, 6);
  EXPECT_EQ(Readout.Counters.PixelErrors, 1);
  EXPECT_EQ(Readout.Counters.Geom.RingMappingErrors, 1);
  EXPECT_EQ(
      Readout.Stats.getValueByName(Parser::METRIC_EVENTS_TIMESTAMP_TOF_HIGH),
      1);
  EXPECT_EQ(Readout.Stats.getValueByName(
                Parser::METRIC_EVENTS_TIMESTAMP_PREVTOF_NEGATIVE),
            1);

  EXPECT_NE(Readout.getInputCounters().RxIdle, 0);
  EXPECT_NE(Readout.Counters.ProcessingIdle, 0);
  Readout.stopThreads();
}

TEST_F(CaenBaseTest, DataReceiveGoodBifrostForceUpdate) {
  XTRACE(DATA, DEB, "Running DataReceiveGood test");
  Settings.DetectorName = "bifrost";
  Settings.ConfigFile = BIFROST_CONFIG;
  Settings.CalibFile = BIFROST_CALIB;
  Settings.UpdateIntervalSec = 0;
  Caen::CaenBase Readout(Settings, DetectorType::BIFROST);

  writePacketToRxFIFO(Readout, TestPacket2);

  EXPECT_EQ(Readout.Stats.getValueByName(
                ESSReadout::Parser::METRIC_PARSER_ESSHEADER_ERRORS_TYPE),
            1);
  EXPECT_EQ(Readout.Counters.Parser.Readouts, 0);
  Readout.stopThreads();
}

TEST_F(CaenBaseTest, DataReceiveGoodMiraclesForceUpdate) {
  XTRACE(DATA, DEB, "Running DataReceiveGood test");
  Settings.DetectorName = "miracles";
  Settings.ConfigFile = MIRACLES_CONFIG;
  Settings.CalibFile = MIRACLES_CALIB;
  Settings.UpdateIntervalSec = 0;
  Caen::CaenBase Readout(Settings, DetectorType::MIRACLES);

  writePacketToRxFIFO(Readout, TestPacket2);

  EXPECT_EQ(
      Readout.Stats.getValueByName(Parser::METRIC_PARSER_ESSHEADER_ERRORS_TYPE),
      1);
  EXPECT_EQ(Readout.Counters.Parser.Readouts, 0);
  Readout.stopThreads();
}

TEST_F(CaenBaseTest, EmulateFIFOError) {
  Caen::CaenBase Readout(Settings, DetectorType::LOKI);
  EXPECT_EQ(Readout.getInputCounters().FifoSeqErrors, 0);

  Readout.startThreads();

  unsigned int rxBufferIndex = Readout.RxRingbuffer.getDataIndex();
  ASSERT_EQ(rxBufferIndex, 0);

  Readout.RxRingbuffer.setDataLength(rxBufferIndex, 0); ///< invalid size

  ASSERT_TRUE(Readout.InputFifo.push(rxBufferIndex));
  Readout.RxRingbuffer.getNextBuffer();

  waitForProcessing(Readout);

  EXPECT_EQ(Readout.getInputCounters().FifoSeqErrors, 1);
  Readout.stopThreads();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
