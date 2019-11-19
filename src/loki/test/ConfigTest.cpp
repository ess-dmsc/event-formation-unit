/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <loki/geometry/Config.h>
#include <test/TestBase.h>
#include <test/SaveBuffer.h>

std::string notjson = R"(
{
  Ceci n’est pas Json
)";

using namespace Loki;

class ConfigTest : public TestBase {
protected:
  Config config;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ConfigTest, Constructor) {
  ASSERT_EQ(config.Panels.size(), 0);
}

TEST_F(ConfigTest, NoConfigFile) {
  ASSERT_THROW(config = Config(""), std::runtime_error);
}

TEST_F(ConfigTest, NotJson) {
  ASSERT_THROW(config = Config("/this_file_doesnt_exist"), std::runtime_error);
}

TEST_F(ConfigTest, InvalidJson) {
  ASSERT_THROW(config = Config("/tmp"), std::runtime_error);
}


int main(int argc, char **argv) {
  std::string filename{"deleteme_notjson.json"};
  saveBuffer(filename, (void *)notjson.c_str(), notjson.size());
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
