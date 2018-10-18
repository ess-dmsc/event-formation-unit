/** Copyright (C) 2018 European Spallation Source ERIC */

#include <common/DataSave.h>
#include <multiblade/mbcommon/MBConfig.h>
#include <multiblade/mbcommon/MBConfigTestData.h>
#include <test/TestBase.h>


class MBConfigTest : public TestBase {};

/** Test cases below */
TEST_F(MBConfigTest, Constructor) {
  Multiblade::MBConfig mbconf;
  ASSERT_FALSE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.getTimeTickNS(), 16);
  ASSERT_EQ(mbconf.getDetector(), nullptr);
  ASSERT_EQ(mbconf.getInstrument(), Multiblade::MBConfig::InstrumentGeometry::Estia);
  ASSERT_TRUE(mbconf.getConfigFile().empty());
  ASSERT_EQ(mbconf.getDigitisers().size(), 0);
}

TEST_F(MBConfigTest, NoFile) {
  Multiblade::MBConfig mbconf;
  ASSERT_THROW(mbconf = Multiblade::MBConfig(""), std::runtime_error);
}


TEST_F(MBConfigTest, InvalidJSON) {
  std::string filename{"deleteme_invalid.json"};
  DataSave tempfile(filename, (void *)invalidjson.c_str(), invalidjson.size());
  Multiblade::MBConfig mbconf;
  ASSERT_THROW(mbconf = Multiblade::MBConfig(filename), std::runtime_error);
}

TEST_F(MBConfigTest, IncompleteConfigFile) {
  std::string filename{"deleteme_incomplete.json"};
  DataSave tempfile(filename, (void *)incompleteconfig.c_str(), incompleteconfig.size());
  Multiblade::MBConfig mbconf;
  ASSERT_THROW(mbconf = Multiblade::MBConfig(filename), std::runtime_error);
}

TEST_F(MBConfigTest, InvaldInstrument) {
  std::string filename{"deleteme_incomplete.json"};
  DataSave tempfile(filename, (void *)invalidinstrument.c_str(), invalidinstrument.size());
  Multiblade::MBConfig mbconf;
  ASSERT_THROW(mbconf = Multiblade::MBConfig(filename), std::runtime_error);
}

TEST_F(MBConfigTest, InvalidDigitiserInFile) {
  std::string filename{"deleteme_invaliddigitiser.json"};
  DataSave tempfile(filename, (void *)invaliddigitiser.c_str(), invaliddigitiser.size());
  Multiblade::MBConfig mbconf;
  ASSERT_THROW(mbconf = Multiblade::MBConfig(filename), std::runtime_error);
}

TEST_F(MBConfigTest, UnknownInstrument) {
  std::string filename{"deleteme_unknowninstrument.json"};
  DataSave tempfile(filename, (void *)unknowninstrument.c_str(), unknowninstrument.size());
  Multiblade::MBConfig mbconf;
  mbconf = Multiblade::MBConfig(filename);
  ASSERT_TRUE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.getInstrument(), Multiblade::MBConfig::InstrumentGeometry::Estia);
}

TEST_F(MBConfigTest, InstrumentFreia) {
  std::string filename{"deleteme_instrumentfreia.json"};
  DataSave tempfile(filename, (void *)instrumentfreia.c_str(), instrumentfreia.size());
  Multiblade::MBConfig mbconf;
  mbconf = Multiblade::MBConfig(filename);
  ASSERT_TRUE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.getInstrument(), Multiblade::MBConfig::InstrumentGeometry::Freia);
}

TEST_F(MBConfigTest, ValidConfigFile) {
  std::string filename{"deleteme_valid.json"};
  DataSave tempfile(filename, (void *)validconfig.c_str(), validconfig.size());
  Multiblade::MBConfig mbconf;
  mbconf = Multiblade::MBConfig(filename);
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
