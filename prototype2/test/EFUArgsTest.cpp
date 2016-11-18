/** Copyright (C) 2016 European Spallation Source ERIC */

#include "TestBase.h"
#include <common/EFUArgs.h>

using namespace std;

class EFUArgsTest : public TestBase { };


TEST_F(EFUArgsTest, Constructor) {
  EFUArgs opts(0, NULL);

  ASSERT_STREQ("localhost:9092", opts.broker.c_str());
  ASSERT_EQ(12, opts.cpustart);
  ASSERT_STREQ("nmx", opts.det.c_str());
  ASSERT_STREQ("0.0.0.0", opts.ip_addr.c_str());
  ASSERT_EQ(9000, opts.port);
  ASSERT_EQ(0xffffffffU, opts.reportmask);
  ASSERT_EQ(0xffffffffU, opts.stopafter);
}

TEST_F(EFUArgsTest, VerifyCommandLineOptions) {
  int argc = 15;
  // clang-format off
  const char *argv[] = {"progname",
                        "-b", "mybroker:myport",
                        "-c" , "99",
                        "-d", "myinst",
                        "-i", "1.2.3.4",
                        "-p", "9876",
                        "-r", "43",
                        "-s", "5" };
  // clang-format on
  EFUArgs opts(argc, (char**)argv);

  ASSERT_STREQ("mybroker:myport", opts.broker.c_str());
  ASSERT_EQ(99, opts.cpustart);
  ASSERT_STREQ("myinst", opts.det.c_str());
  ASSERT_STREQ("1.2.3.4", opts.ip_addr.c_str());
  ASSERT_EQ(9876, opts.port);
  ASSERT_EQ(43, opts.reportmask);
  ASSERT_EQ(5, opts.stopafter);
}

TEST_F(EFUArgsTest, HelpText) {
  int argc = 2;
  const char * argv[] = { "progname",
                          "-h" };

  EFUArgs opts(argc, (char**)argv);
  ASSERT_EQ(argc, 2);
  ASSERT_TRUE(argv != NULL);
}

int main(int argc,char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
