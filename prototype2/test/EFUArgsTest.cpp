/** Copyright (C) 2016 European Spallation Source ERIC */

#include "TestBase.h"
#include <common/EFUArgs.h>

using namespace std;

class EFUArgsTest : public TestBase { };


TEST_F(EFUArgsTest, Constructor) {
  int argc = 13;
  // clang-format off
  const char *argv[] = {"progname",
                        "-b", "mybroker:myport",
                        "-c" , "99",
                        "-d", "myinst",
                        "-i", "1.2.3.4",
                        "-r", "43",
                        "-s", "5" };
  // clang-format on
  EFUArgs opts(argc, (char**)argv);

  ASSERT_STREQ("mybroker:myport", opts.broker.c_str());
  ASSERT_EQ(99, opts.cpustart);
  ASSERT_STREQ("myinst", opts.det.c_str());
  ASSERT_STREQ("1.2.3.4", opts.ip_addr.c_str());
  ASSERT_EQ(43, opts.reportmask);
  ASSERT_EQ(5, opts.stopafter);

}


int main(int argc,char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
