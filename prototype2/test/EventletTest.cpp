/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include "TestBase.h"
#include <NMX/Eventlet.h>
#include <string>
#include <unistd.h>

class EventletTest : public TestBase {};

TEST_F(EventletTest, Constructor) { ASSERT_EQ(1, 1); }

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
