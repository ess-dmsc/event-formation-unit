/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/EFUArgs.h>
#include <test/TestBase.h>

class EFUArgsTest : public TestBase {};

TEST_F(EFUArgsTest, Constructor) {
  EFUArgs efu_args;
  auto settings = efu_args.GetBaseSettings();

  // ASSERT_EQ(12, settings.cpustart); /**< todo fixme */
  ASSERT_EQ("0.0.0.0", settings.DetectorAddress);
  ASSERT_EQ(9000, settings.DetectorPort);
  ASSERT_EQ("localhost:9092", settings.KafkaBroker);
  ASSERT_EQ("127.0.0.1", settings.GraphiteAddress);
  ASSERT_EQ(2003, settings.GraphitePort);
  ASSERT_EQ(0xffffffffU, settings.StopAfterSec);
}

TEST_F(EFUArgsTest, VerifyCommandLineOptions) {

  // clang-format off
  const char *myargv[] = {"progname",
                        "-b", "mybroker:9091",
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
  auto ret = efu_args.parseAndProceed(myargc, (char **)myargv);
  ASSERT_EQ(ret, true); // has detector
  auto settings = efu_args.GetBaseSettings();
  auto glsettings = efu_args.getGraylogSettings();

  ASSERT_EQ("mybroker:9091", settings.KafkaBroker);
  // ASSERT_EQ(99, opts.cpustart); /**< todo fixme */
  ASSERT_EQ("myinst", settings.DetectorPluginName);
  ASSERT_EQ("1.2.3.4", settings.DetectorAddress);
  ASSERT_EQ(9876, settings.DetectorPort);
  ASSERT_EQ("4.3.2.1", settings.GraphiteAddress);
  ASSERT_EQ(2323, settings.GraphitePort);
  ASSERT_EQ(5, settings.StopAfterSec);
  ASSERT_EQ("10.0.0.1", glsettings.address);
  ASSERT_EQ(8989, settings.CommandServerPort);
}

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

TEST_F(EFUArgsTest, HelpText) {
  int myargc = 2;
  const char *myargv[] = {"progname", "-h"};

  EFUArgs efu_args;
  auto ret = efu_args.parseAndProceed(myargc, (char **)myargv);
  ASSERT_EQ(ret, false); // has detector

  ASSERT_EQ(myargc, 2);
  ASSERT_TRUE(myargv != NULL);
}

TEST_F(EFUArgsTest, CoreAffinityOption) {
  int myargc = 5;
  const char *myargv[] = {"progname", "-d", "dummydetector", "-c", "thread1:5"};

  EFUArgs efu_args;
  auto ret = efu_args.parseAndProceed(myargc, (char **)myargv);
  ASSERT_EQ(ret, false); // has detector

  ASSERT_EQ(myargc, 5);
  ASSERT_TRUE(myargv != NULL);
}

TEST_F(EFUArgsTest, PrintHelpText) {
  EFUArgs efu_args;
  MESSAGE() << "This is not a test, just ensuring printout is done once\n" ;
  efu_args.printHelp();
}

TEST_F(EFUArgsTest, XTRACE_Text) {
  EFUArgs efu_args;
  MESSAGE() << "This is not a test, just ensuring printout is done once\n" ;
  efu_args.printSettings();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
