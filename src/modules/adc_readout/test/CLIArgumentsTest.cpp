/** Copyright (C) 2018-2020 European Spallation Source ERIC */

/** @file
 *
 *  \brief Tests of command line arguments.
 */

#include <adc_readout/AdcSettings.h>
#include <gtest/gtest.h>

class CLITesting : public ::testing::Test {
public:
  void SetUp() override { setCLIArguments(TestApp, ReadoutSettings); }
  AdcSettings ReadoutSettings;
  CLI::App TestApp;
};

TEST_F(CLITesting, DefaultValuesTest) {
  std::vector<std::string> TestArguments{"AppName"};
  std::vector<char *> CArgs;
  CArgs.reserve(TestArguments.size());

  std::transform(TestArguments.begin(), TestArguments.end(),
                 std::back_inserter(CArgs),
                 [](auto &Arg) { return const_cast<char *>(Arg.c_str()); });

  TestApp.parse(CArgs.size(), &CArgs[0]);
  EXPECT_EQ(ReadoutSettings.PeakDetection, false);
  EXPECT_EQ(ReadoutSettings.SerializeSamples, false);
  EXPECT_EQ(ReadoutSettings.SampleTimeStamp, false);
  EXPECT_EQ(ReadoutSettings.TakeMeanOfNrOfSamples, 1);
  EXPECT_EQ(ReadoutSettings.TimeStampLocation, "Middle");
}

TEST_F(CLITesting, EnableStreaming) {
  std::vector<std::string> TestArguments{"AppName", "--serialize_samples"};
  std::vector<char *> CArgs;
  CArgs.reserve(TestArguments.size());

  std::transform(TestArguments.begin(), TestArguments.end(),
                 std::back_inserter(CArgs),
                 [](auto &Arg) { return const_cast<char *>(Arg.c_str()); });
  TestApp.parse(CArgs.size(), &CArgs[0]);
  EXPECT_EQ(ReadoutSettings.SerializeSamples, true);
}

TEST_F(CLITesting, EnablePeakDetect) {
  std::vector<std::string> TestArguments{"AppName", "--peak_detection"};
  std::vector<char *> CArgs;
  CArgs.reserve(TestArguments.size());

  std::transform(TestArguments.begin(), TestArguments.end(),
                 std::back_inserter(CArgs),
                 [](auto &Arg) { return const_cast<char *>(Arg.c_str()); });
  TestApp.parse(CArgs.size(), &CArgs[0]);
  EXPECT_EQ(ReadoutSettings.PeakDetection, true);
}

TEST_F(CLITesting, EnableSampleTimeStamps) {
  std::vector<std::string> TestArguments{"AppName", "--sample_timestamp"};
  std::vector<char *> CArgs;
  CArgs.reserve(TestArguments.size());

  std::transform(TestArguments.begin(), TestArguments.end(),
                 std::back_inserter(CArgs),
                 [](auto &Arg) { return const_cast<char *>(Arg.c_str()); });
  TestApp.parse(CArgs.size(), &CArgs[0]);
  EXPECT_EQ(ReadoutSettings.SampleTimeStamp, true);
}

TEST_F(CLITesting, MeanOfSamples) {
  std::vector<std::string> TestArguments{"AppName", "--mean_of_samples", "5"};
  std::vector<char *> CArgs;
  CArgs.reserve(TestArguments.size());

  std::transform(TestArguments.begin(), TestArguments.end(),
                 std::back_inserter(CArgs),
                 [](auto &Arg) { return const_cast<char *>(Arg.c_str()); });
  TestApp.parse(CArgs.size(), &CArgs[0]);
  EXPECT_EQ(ReadoutSettings.TakeMeanOfNrOfSamples, 5);
}

TEST_F(CLITesting, MeanOfSamplesFail1) {
  std::vector<std::string> TestArguments{"AppName", "--mean_of_samples", "0"};
  std::vector<char *> CArgs;
  CArgs.reserve(TestArguments.size());

  std::transform(TestArguments.begin(), TestArguments.end(),
                 std::back_inserter(CArgs),
                 [](auto &Arg) { return const_cast<char *>(Arg.c_str()); });
  EXPECT_THROW(TestApp.parse(CArgs.size(), &CArgs[0]), CLI::ParseError);
}

TEST_F(CLITesting, MeanOfSamplesFail2) {
  std::vector<std::string> TestArguments{"AppName", "--mean_of_samples", "-1"};
  std::vector<char *> CArgs;
  CArgs.reserve(TestArguments.size());

  std::transform(TestArguments.begin(), TestArguments.end(),
                 std::back_inserter(CArgs),
                 [](auto &Arg) { return const_cast<char *>(Arg.c_str()); });
  EXPECT_THROW(TestApp.parse(CArgs.size(), &CArgs[0]), CLI::ParseError);
}

TEST_F(CLITesting, MeanOfSamplesFail3) {
  std::vector<std::string> TestArguments{"AppName", "--mean_of_samples",
                                         "hello"};
  std::vector<char *> CArgs;
  CArgs.reserve(TestArguments.size());

  std::transform(TestArguments.begin(), TestArguments.end(),
                 std::back_inserter(CArgs),
                 [](auto &Arg) { return const_cast<char *>(Arg.c_str()); });
  EXPECT_THROW(TestApp.parse(CArgs.size(), &CArgs[0]), CLI::ParseError);
}

TEST_F(CLITesting, TimeStampLocSuccess1) {
  std::vector<std::string> TestArguments{"AppName", "--time_stamp_loc",
                                         "Start"};
  std::vector<char *> CArgs;
  CArgs.reserve(TestArguments.size());

  std::transform(TestArguments.begin(), TestArguments.end(),
                 std::back_inserter(CArgs),
                 [](auto &Arg) { return const_cast<char *>(Arg.c_str()); });
  EXPECT_NO_THROW(TestApp.parse(CArgs.size(), &CArgs[0]));
}

TEST_F(CLITesting, TimeStampLocSuccess2) {
  std::vector<std::string> TestArguments{"AppName", "--time_stamp_loc",
                                         "Middle"};
  std::vector<char *> CArgs;
  CArgs.reserve(TestArguments.size());

  std::transform(TestArguments.begin(), TestArguments.end(),
                 std::back_inserter(CArgs),
                 [](auto &Arg) { return const_cast<char *>(Arg.c_str()); });
  EXPECT_NO_THROW(TestApp.parse(CArgs.size(), &CArgs[0]));
}

TEST_F(CLITesting, TimeStampLocSuccess3) {
  std::vector<std::string> TestArguments{"AppName", "--time_stamp_loc", "End"};
  std::vector<char *> CArgs;
  CArgs.reserve(TestArguments.size());

  std::transform(TestArguments.begin(), TestArguments.end(),
                 std::back_inserter(CArgs),
                 [](auto &Arg) { return const_cast<char *>(Arg.c_str()); });
  EXPECT_NO_THROW(TestApp.parse(CArgs.size(), &CArgs[0]));
}

TEST_F(CLITesting, TimeStampLocFail) {
  std::vector<std::string> TestArguments{"AppName", "--time_stamp_loc",
                                         "start"};
  std::vector<char *> CArgs;
  CArgs.reserve(TestArguments.size());

  std::transform(TestArguments.begin(), TestArguments.end(),
                 std::back_inserter(CArgs),
                 [](auto &Arg) { return const_cast<char *>(Arg.c_str()); });
  EXPECT_THROW(TestApp.parse(CArgs.size(), &CArgs[0]), CLI::ParseError);
}
