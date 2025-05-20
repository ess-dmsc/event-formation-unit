// Copyright (C) 2016 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/JsonFile.h>
#include <common/testutils/TestBase.h>
#include <freia/geometry/Config.h>


using namespace Freia;


class FreiaConfigTest : public TestBase {
protected:
  const double FPEquality{0.000001};

  Config config{"Freia", "config.json"};

  void SetUp() override {
    config.setRoot(from_json_file(FREIA_FULL));
  }

  void TearDown() override {}
};


TEST_F(FreiaConfigTest, Constructor) {
  ASSERT_EQ(config.NumPixels, 0);
  ASSERT_EQ(config.NumHybrids, 0);
}


TEST_F(FreiaConfigTest, UninitialisedHybrids) {
  ASSERT_EQ(config.getHybrid(1, 0, 0).Initialised, false);
  ASSERT_ANY_THROW(config.getHybrid("not a hybrid ID"));
}


TEST_F(FreiaConfigTest, NoDetector) {
  json_change_key(config.root(), "Detector", "NoMoreDetector");
  ASSERT_ANY_THROW(config.applyVMM3Config());
}


TEST_F(FreiaConfigTest, InvalidDetector) {
  config["Detector"] = "Frigg";
  ASSERT_ANY_THROW(config.applyVMM3Config());
}


TEST_F(FreiaConfigTest, InvalidRing) {
  config["Config"][4]["Ring"] = 12;
  ASSERT_ANY_THROW(config.applyVMM3Config());
}


TEST_F(FreiaConfigTest, InvalidConfig) {
  json_change_key(config["Config"][4], "Ring", "Ringo");
  ASSERT_ANY_THROW(config.applyConfig());
}


TEST_F(FreiaConfigTest, Duplicate) {
  config["Config"][3]["Ring"] = config["Config"][0]["Ring"];
  config["Config"][3]["FEN"] = config["Config"][0]["FEN"];
  config["Config"][3]["Hybrid"] = config["Config"][0]["Hybrid"];
  ASSERT_ANY_THROW(config.applyVMM3Config());
}


TEST_F(FreiaConfigTest, BadVersion) {
  config["Version"] = 2;
  ASSERT_ANY_THROW(config.applyConfig());
}


TEST_F(FreiaConfigTest, BadThresholdArraySize) {
  config["Config"][0]["Thresholds"][0].push_back(42);
  ASSERT_ANY_THROW(config.applyConfig());
}


/// Test optional parameters
TEST_F(FreiaConfigTest, ParmMaxGapWire) {
  EXPECT_EQ(config.MBFileParameters.MaxGapWire, 0);

  config["MaxGapWire"] = 129;
  config.applyConfig();
  EXPECT_EQ(config.MBFileParameters.MaxGapWire, 129);
}

TEST_F(FreiaConfigTest, DefaultParmMaxGapWire) {
  json_change_key(config.root(), "MaxGapWire", "NoMaxGapWire");
  EXPECT_EQ(config.MBFileParameters.MaxGapWire, 0);
  config.applyConfig();
  EXPECT_EQ(config.MBFileParameters.MaxGapWire, 0);
}

TEST_F(FreiaConfigTest, ParmMaxGapStrip) {
  EXPECT_EQ(config.MBFileParameters.MaxGapStrip, 0);

  config["MaxGapStrip"] = 129;
  config.applyConfig();
  EXPECT_EQ(config.MBFileParameters.MaxGapStrip, 129);
}

TEST_F(FreiaConfigTest, DefaultParmMaxGapStrip) {
  json_change_key(config.root(), "MaxGapStrip", "NoMaxGapStrip");
  EXPECT_EQ(config.MBFileParameters.MaxGapStrip, 0);
  config.applyConfig();
  EXPECT_EQ(config.MBFileParameters.MaxGapStrip, 0);
}

TEST_F(FreiaConfigTest, ParmSplitMultiEvents) {
  EXPECT_EQ(config.MBFileParameters.SplitMultiEvents, false);

  config["SplitMultiEvents"] = true;
  config.applyConfig();
  EXPECT_EQ(config.MBFileParameters.SplitMultiEvents, true);
}

TEST_F(FreiaConfigTest, ParmMultiEventsCoefficientLow) {
  EXPECT_NEAR(config.MBFileParameters.SplitMultiEventsCoefficientLow, 0.8, FPEquality);

  config["SplitMultiEventsCoefficientLow"] = 0.42;
  config.applyConfig();
  EXPECT_NEAR(config.MBFileParameters.SplitMultiEventsCoefficientLow, 0.42, FPEquality);
}

TEST_F(FreiaConfigTest, ParmMultiEventsCoefficientHigh) {
  EXPECT_NEAR(config.MBFileParameters.SplitMultiEventsCoefficientHigh, 1.2, FPEquality);

  config["SplitMultiEventsCoefficientHigh"] = 0.42;
  config.applyConfig();
  EXPECT_NEAR(config.MBFileParameters.SplitMultiEventsCoefficientHigh, 0.42, FPEquality);
}

TEST_F(FreiaConfigTest, ParmMaxMatchingTimeGap) {
  EXPECT_EQ(config.MBFileParameters.MaxMatchingTimeGap, 500);

  config["MaxMatchingTimeGap"] = 42;
  config.applyConfig();
  EXPECT_EQ(config.MBFileParameters.MaxMatchingTimeGap, 42);
}

TEST_F(FreiaConfigTest, ParmMaxClusteringTimeGap) {
  EXPECT_EQ(config.MBFileParameters.MaxClusteringTimeGap, 500);

  config["MaxClusteringTimeGap"] = 42;
  config.applyConfig();
  EXPECT_EQ(config.MBFileParameters.MaxClusteringTimeGap, 42);
}

// Compare calculated maxpixels and number of fens against
// ICD
struct RingCfg {
  uint8_t Ring;
  uint16_t FEN;
  uint16_t Hybrid;
};

// This table generated from the ICD and will be
// compared to the calculated values
std::vector<RingCfg> ReferenceConfig{
    {0, 0, 0},  {0, 0, 1},  {0, 1, 0}, {0, 1, 1}, {1, 0, 0},  {1, 0, 1},
    {1, 1, 0},  {1, 1, 1},  {2, 0, 0}, {2, 0, 1}, {2, 1, 0},  {2, 1, 1},
    {3, 0, 0},  {3, 0, 1},  {4, 0, 0}, {4, 0, 1}, {5, 0, 0},  {5, 0, 1},
    {6, 0, 0},  {6, 0, 1},  {7, 0, 0}, {7, 0, 1}, {8, 0, 0},  {8, 0, 1},
    {9, 0, 0},  {9, 0, 1},  {9, 1, 0}, {9, 1, 1}, {10, 0, 0}, {10, 0, 1},
    {10, 1, 0}, {10, 1, 1},
};


TEST_F(FreiaConfigTest, FullInstrument) {
  config = Config("Freia", FREIA_FULL);
  config.loadAndApplyConfig();
  ASSERT_EQ(config.NumPixels, 65536);
  ASSERT_EQ(config.NumHybrids, 32);

  for (const auto &Ref : ReferenceConfig) {
    ASSERT_EQ(config.getHybrid(Ref.Ring, Ref.FEN, Ref.Hybrid).Initialised,
              true);
  }
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
