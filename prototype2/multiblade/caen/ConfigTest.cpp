/** Copyright (C) 2018 European Spallation Source ERIC */

#include <common/DataSave.h>
#include <multiblade/caen/Config.h>
#include <multiblade/caen/ConfigTestData.h>
#include <test/TestBase.h>


class ConfigTest : public TestBase {};

/** Test cases below */
TEST_F(ConfigTest, Constructor) {
  Multiblade::Config mbconf;
  ASSERT_FALSE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.getTimeTickNS(), 16);
  ASSERT_EQ(mbconf.getDigitizers(), nullptr);
  ASSERT_EQ(mbconf.getInstrument(), Multiblade::Config::InstrumentGeometry::Estia);
  ASSERT_TRUE(mbconf.getConfigFile().empty());
  ASSERT_EQ(mbconf.getDigitisers().size(), 0);
}

TEST_F(ConfigTest, NoFile) {
  Multiblade::Config mbconf;
  ASSERT_THROW(mbconf = Multiblade::Config(""), std::runtime_error);
}


TEST_F(ConfigTest, InvalidJSON) {
  std::string filename{"deleteme_invalid.json"};
  DataSave tempfile(filename, (void *)invalidjson.c_str(), invalidjson.size());
  Multiblade::Config mbconf;
  ASSERT_THROW(mbconf = Multiblade::Config(filename), std::runtime_error);
}

TEST_F(ConfigTest, IncompleteConfigFile) {
  std::string filename{"deleteme_incomplete.json"};
  DataSave tempfile(filename, (void *)incompleteconfig.c_str(), incompleteconfig.size());
  Multiblade::Config mbconf;
  ASSERT_THROW(mbconf = Multiblade::Config(filename), std::runtime_error);
}

TEST_F(ConfigTest, InvaldInstrument) {
  std::string filename{"deleteme_incomplete.json"};
  DataSave tempfile(filename, (void *)invalidinstrument.c_str(), invalidinstrument.size());
  Multiblade::Config mbconf;
  ASSERT_THROW(mbconf = Multiblade::Config(filename), std::runtime_error);
}

TEST_F(ConfigTest, InvalidDigitiserInFile) {
  std::string filename{"deleteme_invaliddigitiser.json"};
  DataSave tempfile(filename, (void *)invaliddigitiser.c_str(), invaliddigitiser.size());
  Multiblade::Config mbconf;
  ASSERT_THROW(mbconf = Multiblade::Config(filename), std::runtime_error);
}

TEST_F(ConfigTest, UnknownInstrument) {
  std::string filename{"deleteme_unknowninstrument.json"};
  DataSave tempfile(filename, (void *)unknowninstrument.c_str(), unknowninstrument.size());
  Multiblade::Config mbconf;
  mbconf = Multiblade::Config(filename);
  ASSERT_TRUE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.getInstrument(), Multiblade::Config::InstrumentGeometry::Estia);
}

TEST_F(ConfigTest, InstrumentFreia) {
  std::string filename{"deleteme_instrumentfreia.json"};
  DataSave tempfile(filename, (void *)instrumentfreia.c_str(), instrumentfreia.size());
  Multiblade::Config mbconf;
  mbconf = Multiblade::Config(filename);
  ASSERT_TRUE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.getInstrument(), Multiblade::Config::InstrumentGeometry::Freia);
}

TEST_F(ConfigTest, ValidConfigFile) {
  std::string filename{"deleteme_valid.json"};
  DataSave tempfile(filename, (void *)validconfig.c_str(), validconfig.size());
  Multiblade::Config mbconf;
  mbconf = Multiblade::Config(filename);
  ASSERT_TRUE(mbconf.isConfigLoaded());
  ASSERT_EQ(mbconf.getTimeTickNS(), 17);
  ASSERT_NE(mbconf.getDigitizers(), nullptr);
  ASSERT_EQ(mbconf.getConfigFile(), filename);
  ASSERT_EQ(mbconf.getDigitisers().size(), 6);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
