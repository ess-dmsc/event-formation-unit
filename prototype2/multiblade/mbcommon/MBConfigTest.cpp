/** Copyright (C) 2018 European Spallation Source ERIC */

#include <common/DataSave.h>
#include <multiblade/mbcommon/MBConfig.h>
#include <multiblade/mbcommon/MBConfigTestData.h>
#include <test/TestBase.h>

class MBConfigTest : public TestBase {};

/** Test cases below */
TEST_F(MBConfigTest, Constructor) {
  MBConfig mbconf;
  ASSERT_FALSE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.getTimeTickNS(), 16);
  ASSERT_EQ(mbconf.getDetector(), nullptr);
  ASSERT_EQ(mbconf.getInstrument(), MBConfig::InstrumentGeometry::Estia);
  ASSERT_TRUE(mbconf.getConfigFile().empty());
  ASSERT_EQ(mbconf.getDigitisers().size(), 0);
}

TEST_F(MBConfigTest, NoFile) {
  MBConfig mbconf;
  mbconf = MBConfig("");
  ASSERT_FALSE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.getTimeTickNS(), 16);
  ASSERT_NE(mbconf.getDetector(), nullptr);
  ASSERT_TRUE(mbconf.getConfigFile().empty());
  ASSERT_EQ(mbconf.getDigitisers().size(), 0);
}


TEST_F(MBConfigTest, InvalidJSON) {
  std::string filename{"deleteme_invalid.json"};
  DataSave tempfile(filename, (void *)invalidjson.c_str(), invalidjson.size());
  MBConfig mbconf;
  mbconf = MBConfig(filename);
  ASSERT_FALSE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.getTimeTickNS(), 16);
  ASSERT_NE(mbconf.getDetector(), nullptr);
  ASSERT_EQ(mbconf.getConfigFile(), filename);
  ASSERT_EQ(mbconf.getDigitisers().size(), 0);
}

TEST_F(MBConfigTest, IncompleteConfigFile) {
  std::string filename{"deleteme_incomplete.json"};
  DataSave tempfile(filename, (void *)incompleteconfig.c_str(), incompleteconfig.size());
  MBConfig mbconf;
  mbconf = MBConfig(filename);
  ASSERT_FALSE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.getTimeTickNS(), 16);
  ASSERT_NE(mbconf.getDetector(), nullptr);
  ASSERT_EQ(mbconf.getConfigFile(), filename);
  ASSERT_EQ(mbconf.getDigitisers().size(), 0);
}

TEST_F(MBConfigTest, InvalidDigitiserInFile) {
  std::string filename{"deleteme_invaliddigitiser.json"};
  DataSave tempfile(filename, (void *)invaliddigitiser.c_str(), invaliddigitiser.size());
  MBConfig mbconf;
  mbconf = MBConfig(filename);
  ASSERT_FALSE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.getTimeTickNS(), 16);
  ASSERT_NE(mbconf.getDetector(), nullptr);
  ASSERT_EQ(mbconf.getConfigFile(), filename);
  ASSERT_EQ(mbconf.getDigitisers().size(), 0);
}

TEST_F(MBConfigTest, ValidConfigFile) {
  std::string filename{"deleteme_valid.json"};
  DataSave tempfile(filename, (void *)validconfig.c_str(), validconfig.size());
  MBConfig mbconf;
  mbconf = MBConfig(filename);
  ASSERT_TRUE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.getTimeTickNS(), 17);
  ASSERT_NE(mbconf.getDetector(), nullptr);
  ASSERT_EQ(mbconf.getConfigFile(), filename);
  ASSERT_EQ(mbconf.getDigitisers().size(), 6);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
