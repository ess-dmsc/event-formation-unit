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
  ASSERT_EQ(mbconf.TimeTickNS, 16);
  ASSERT_EQ(mbconf.detector, nullptr);
  ASSERT_EQ(mbconf.instrument, MBConfig::InstrumentGeometry::Estia);
  ASSERT_TRUE(mbconf.ConfigFile.empty());
  ASSERT_EQ(mbconf.Digitisers.size(), 0);
}

TEST_F(MBConfigTest, NoFile) {
  MBConfig mbconf;
  mbconf = MBConfig("");
  ASSERT_FALSE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.TimeTickNS, 16);
  ASSERT_NE(mbconf.detector, nullptr);
  ASSERT_TRUE(mbconf.ConfigFile.empty());
  ASSERT_EQ(mbconf.Digitisers.size(), 0);
}


TEST_F(MBConfigTest, InvalidJSON) {
  std::string filename{"deleteme_invalid.json"};
  DataSave tempfile(filename, (void *)invalidjson.c_str(), invalidjson.size());
  MBConfig mbconf;
  mbconf = MBConfig(filename);
  ASSERT_FALSE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.TimeTickNS, 16);
  ASSERT_NE(mbconf.detector, nullptr);
  ASSERT_EQ(mbconf.ConfigFile, filename);
  ASSERT_EQ(mbconf.Digitisers.size(), 0);
}

TEST_F(MBConfigTest, IncompleteConfigFile) {
  std::string filename{"deleteme_incomplete.json"};
  DataSave tempfile(filename, (void *)incompleteconfig.c_str(), incompleteconfig.size());
  MBConfig mbconf;
  mbconf = MBConfig(filename);
  ASSERT_FALSE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.TimeTickNS, 16);
  ASSERT_NE(mbconf.detector, nullptr);
  ASSERT_EQ(mbconf.ConfigFile, filename);
  ASSERT_EQ(mbconf.Digitisers.size(), 0);
}

TEST_F(MBConfigTest, ValidConfigFile) {
  std::string filename{"deleteme_valid.json"};
  DataSave tempfile(filename, (void *)validconfig.c_str(), validconfig.size());
  MBConfig mbconf;
  mbconf = MBConfig(filename);
  ASSERT_TRUE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.TimeTickNS, 17);
  ASSERT_NE(mbconf.detector, nullptr);
  ASSERT_EQ(mbconf.ConfigFile, filename);
  ASSERT_EQ(mbconf.Digitisers.size(), 6);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
