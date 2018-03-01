/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Tests of command line arguments.
 */

#include <gtest/gtest.h>
#include "../AdcSettings.h"

class CLITesting : public ::testing::Test {
public:
  virtual void SetUp() {
    SetCLIArguments(TestApp, AdcSettings);
  }
  AdcSettingsStruct AdcSettings;
  CLI::App TestApp;
};

TEST_F(CLITesting, DefaultValuesTest) {
  std::vector<std::string> TestArguments{"AppName"};
  std::vector<char*> CArgs;
  CArgs.reserve(TestArguments.size());
  
  for (int i = 0; i < TestArguments.size(); ++i) {
    CArgs.push_back(const_cast<char*>(TestArguments[i].c_str()));
  }
  TestApp.parse(CArgs.size(), &CArgs[0]);
}

