/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/EFUArgs.h>
#include <test/TestBase.h>

class EFUArgsTest : public TestBase {};

TEST_F(EFUArgsTest, Constructor) {
  EFUArgs opts(0, NULL);

  ASSERT_STREQ("localhost:9092", opts.broker.c_str());
  ASSERT_EQ(12, opts.cpustart);
  ASSERT_STREQ("cspec", opts.det.c_str());
  ASSERT_STREQ("0.0.0.0", opts.ip_addr.c_str());
  ASSERT_EQ(9000, opts.port);
  ASSERT_STREQ("127.0.0.1", opts.graphite_ip_addr.c_str());
  ASSERT_EQ(2003, opts.graphite_port);
  ASSERT_EQ(0x2U, opts.reportmask);
  ASSERT_EQ(0xffffffffU, opts.stopafter);
}

TEST_F(EFUArgsTest, VerifyCommandLineOptions) {

  // clang-format off
  const char *myargv[] = {"progname",
                        "-b", "mybroker:myport",
                        "-c" , "99",
                        "-d", "myinst",
                        "-i", "1.2.3.4",
                        "-p", "9876",
                        "-r", "43",
                        "-g", "4.3.2.1",
                        "-o", "2323",
                        "-s", "5",
                        "-a", "10.0.0.1",
                        "-m", "8989" };
  // clang-format on
  int myargc = 23;
  EFUArgs opts(myargc, (char **)myargv);

  ASSERT_STREQ("mybroker:myport", opts.broker.c_str());
  ASSERT_EQ(99, opts.cpustart);
  ASSERT_STREQ("myinst", opts.det.c_str());
  ASSERT_STREQ("1.2.3.4", opts.ip_addr.c_str());
  ASSERT_EQ(9876, opts.port);
  ASSERT_EQ(43, opts.reportmask);
  ASSERT_STREQ("4.3.2.1", opts.graphite_ip_addr.c_str());
  ASSERT_EQ(2323, opts.graphite_port);
  ASSERT_EQ(5, opts.stopafter);
  ASSERT_STREQ("10.0.0.1", opts.graylog_ip.c_str());
  ASSERT_EQ(8989, opts.cmdserver_port);
}

TEST_F(EFUArgsTest, HelpText) {
  int myargc = 2;
  const char *myargv[] = {"progname", "-h"};

  EFUArgs opts2(myargc, (char **)myargv);
  ASSERT_EQ(myargc, 2);
  ASSERT_TRUE(myargv != NULL);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
