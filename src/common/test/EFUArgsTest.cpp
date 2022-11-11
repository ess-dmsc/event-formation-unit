/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#ifndef __llvm__
#if __GNUC__ == 4
#pragma message(                                                               \
    "GCC4 does not implement regular expressions. Ignoring some unit tests.")
#else
#define RUN_REGEX_UNIT_TESTS
#endif
#else
#define RUN_REGEX_UNIT_TESTS
#endif

#ifdef RUN_REGEX_UNIT_TESTS
#pragma message("Unit tests testing regular expressions are enabled.")
#endif

#include <common/detector/EFUArgs.h>
#include <common/testutils/TestBase.h>

class EFUArgsTest : public TestBase {};

TEST_F(EFUArgsTest, ExitOnHelp) {
  const char *myargv[] = {
      "someName",
      "-h",
  };
  int myargc = 2;
  EFUArgs Args;
  EXPECT_EQ(Args.parseArgs(myargc, (char **)myargv), EFUArgs::Status::EXIT);
}

TEST_F(EFUArgsTest, Constructor) {
  EFUArgs efu_args;
  auto settings = efu_args.getBaseSettings();

  // ASSERT_EQ(12, settings.cpustart); /**< todo fixme */
  EXPECT_EQ("0.0.0.0", settings.DetectorAddress);
  EXPECT_EQ(9000, settings.DetectorPort);
  EXPECT_EQ("localhost:9092", settings.KafkaBroker);
  EXPECT_EQ("127.0.0.1", settings.GraphiteAddress);
  EXPECT_EQ(2003, settings.GraphitePort);
  EXPECT_EQ(0xffffffffU, settings.StopAfterSec);
}

TEST_F(EFUArgsTest, VerifyCommandLineOptions) {

  // clang-format off
  const char *myargv[] = {"progname",
                        "-b", "mybroker",
                        "-i", "1.2.3.4",
                        "-p", "9876",
                        "-g", "4.3.2.1",
                        "-o", "2323",
                        "-s", "5",
                        "-a", "10.0.0.1",
                        "-m", "8989" };
  // clang-format on
  int myargc = 17;
  EFUArgs efu_args;
  auto ret = efu_args.parseArgs(myargc, (char **)myargv);
  ASSERT_EQ(ret, EFUArgs::Status::CONTINUE); // has detector
  auto settings = efu_args.getBaseSettings();
  auto glsettings = efu_args.getGraylogSettings();

  ASSERT_EQ("mybroker", settings.KafkaBroker);
  ASSERT_EQ("1.2.3.4", settings.DetectorAddress);
  ASSERT_EQ(9876, settings.DetectorPort);
  ASSERT_EQ("4.3.2.1", settings.GraphiteAddress);
  ASSERT_EQ(2323, settings.GraphitePort);
  ASSERT_EQ(5, settings.StopAfterSec);
  ASSERT_EQ("10.0.0.1", glsettings.address);
  ASSERT_EQ(8989, settings.CommandServerPort);
}

TEST_F(EFUArgsTest, HelpText) {
  int myargc = 2;
  const char *myargv[] = {"progname", "-h"};

  EFUArgs efu_args;
  auto ret = efu_args.parseArgs(myargc, (char **)myargv);
  ASSERT_EQ(ret, EFUArgs::Status::EXIT); // has detector

  ASSERT_EQ(myargc, 2);
  ASSERT_TRUE(myargv != NULL);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
