/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#ifndef __llvm__
#if __GNUC__ == 4
#pragma message("GCC4 does not implement regular expressions. Ignoring some unit tests.")
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
  const char *myargv[] = {"someName", "-h",};
    int myargc = 2;
  EFUArgs Args;
  EXPECT_EQ(Args.parseFirstPass(myargc, (char**)myargv), EFUArgs::Status::EXIT);
}

TEST_F(EFUArgsTest, DoNotExitOnHelp) {
  const char *myargv[] = {"someName", "-h", "-d some_det"};
  int myargc = 3;
  EFUArgs Args;
  EXPECT_EQ(Args.parseFirstPass(myargc, (char**)myargv), EFUArgs::Status::CONTINUE);
}

TEST_F(EFUArgsTest, ExitOnNoArgs) {
  const char *myargv[] = {"someName"};
  int myargc = 1;
  EFUArgs Args;
  EXPECT_EQ(Args.parseFirstPass(myargc, (char**)myargv), EFUArgs::Status::EXIT);
}

TEST_F(EFUArgsTest, IgnoreFirstPassFailure) {
  const char *myargv[] = {"someName", "-h", "-d", "some_det", "-p", "hej"};
  int myargc = 6;
  EFUArgs Args;
  EXPECT_EQ(Args.parseFirstPass(myargc, (char**)myargv), EFUArgs::Status::CONTINUE);
}

TEST_F(EFUArgsTest, SecondPassExitOnHelp) {
  const char *myargv[] = {"someName", "-h"};
  int myargc = 2;
  EFUArgs Args;
  EXPECT_EQ(Args.parseSecondPass(myargc, (char**)myargv), EFUArgs::Status::EXIT);
}

TEST_F(EFUArgsTest, SecondPassExitOnHelpAndDet) {
  const char *myargv[] = {"someName", "-h", "-d", "some_det"};
  int myargc = 4;
  EFUArgs Args;
  EXPECT_EQ(Args.parseSecondPass(myargc, (char**)myargv), EFUArgs::Status::EXIT);
}

TEST_F(EFUArgsTest, SecondPassExitOnFailure) {
  const char *myargv[] = {"someName",};
  int myargc = 1;
  EFUArgs Args;
  EXPECT_EQ(Args.parseSecondPass(myargc, (char**)myargv), EFUArgs::Status::EXIT);
}

TEST_F(EFUArgsTest, SecondPassContinue) {
  const char *myargv[] = {"someName", "-d", "some_det"};
  int myargc = 3;
  EFUArgs Args;
  EXPECT_EQ(Args.parseSecondPass(myargc, (char**)myargv), EFUArgs::Status::CONTINUE);
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
                        "-c" , "99",
                        "-d", "myinst",
                        "-i", "1.2.3.4",
                        "-p", "9876",
                        "-g", "4.3.2.1",
                        "-o", "2323",
                        "-s", "5",
                        "-a", "10.0.0.1",
                        "-m", "8989" };
  // clang-format on
  int myargc = 21;
  EFUArgs efu_args;
  auto ret = efu_args.parseSecondPass(myargc, (char **)myargv);
  ASSERT_EQ(ret, EFUArgs::Status::CONTINUE); // has detector
  auto settings = efu_args.getBaseSettings();
  auto glsettings = efu_args.getGraylogSettings();

  ASSERT_EQ("mybroker", settings.KafkaBroker);
  // ASSERT_EQ(99, opts.cpustart); /**< todo fixme */
  ASSERT_EQ("myinst", efu_args.getDetectorName());
  ASSERT_EQ("1.2.3.4", settings.DetectorAddress);
  ASSERT_EQ(9876, settings.DetectorPort);
  ASSERT_EQ("4.3.2.1", settings.GraphiteAddress);
  ASSERT_EQ(2323, settings.GraphitePort);
  ASSERT_EQ(5, settings.StopAfterSec);
  ASSERT_EQ("10.0.0.1", glsettings.address);
  ASSERT_EQ(8989, settings.CommandServerPort);
}

#ifdef RUN_REGEX_UNIT_TESTS
TEST_F(EFUArgsTest, CoreAffinityOption) {
  int myargc = 5;
  const char *myargv[] = {"progname", "-d", "dummydetector", "-c", "thread1:5"};
  
  EFUArgs efu_args;
  auto ret = efu_args.parseFirstPass(myargc, (char **)myargv);
  ASSERT_EQ(ret, EFUArgs::Status::CONTINUE); // has detector
  
  ASSERT_EQ(myargc, 5);
  ASSERT_TRUE(myargv != NULL);
}

TEST_F(EFUArgsTest, CoreAffinityOptionFailure) {
  int myargc = 5;
  const char *myargv[] = {"progname", "-d", "dummydetector", "-c", "thread1:h"};
  
  EFUArgs efu_args;
  auto ret = efu_args.parseSecondPass(myargc, (char **)myargv);
  ASSERT_EQ(ret, EFUArgs::Status::EXIT); // has detector
  
  ASSERT_EQ(myargc, 5);
  ASSERT_TRUE(myargv != NULL);
}
#endif

TEST_F(EFUArgsTest, HelpText) {
  int myargc = 2;
  const char *myargv[] = {"progname", "-h"};

  EFUArgs efu_args;
  auto ret = efu_args.parseFirstPass(myargc, (char **)myargv);
  ASSERT_EQ(ret, EFUArgs::Status::EXIT); // has detector

  ASSERT_EQ(myargc, 2);
  ASSERT_TRUE(myargv != NULL);
}

TEST_F(EFUArgsTest, StoreConfigFile) {
  //Delete contents of possible file
  std::ofstream OutTestFile("ConfigFile.ini", std::ios::binary | std::ios::trunc);
  OutTestFile.close();
  const char* Args[] = {"progname", "-d", "some_det", "--write_config", "ConfigFile.ini"};
  EFUArgs efu_args;
  efu_args.parseSecondPass(5, (char**)Args);
  std::ifstream InTestFile("ConfigFile.ini", std::ios::binary);
  std::string FileContents((std::istreambuf_iterator<char>(InTestFile)),
                  std::istreambuf_iterator<char>());
  EXPECT_NE(FileContents.find("det=\"some_det\""), std::string::npos);
}

TEST_F(EFUArgsTest, LoadConfigFile) {
  const char* Args1[] = {"progname", "-d", "no_real_detector_name", "--write_config", "ConfigB.ini"};
  EFUArgs efu_args;
  EXPECT_EQ(efu_args.parseSecondPass(5, (char**)Args1), EFUArgs::Status::EXIT);
  
  const char* Args2[] = {"progname", "--read_config", "ConfigB.ini"};
  EFUArgs efu_args2;
  EXPECT_EQ(efu_args2.parseSecondPass(3, (char**)Args2), EFUArgs::Status::CONTINUE);
  EXPECT_EQ(efu_args2.getDetectorName(), "no_real_detector_name");
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
