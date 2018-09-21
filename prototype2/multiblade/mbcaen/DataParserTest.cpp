/** Copyright (C) 2017-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <multiblade/mbcaen/DataParser.h>
#include <multiblade/mbcaen/TestData.h>
#include <test/TestBase.h>

class MultibladeDataTest : public TestBase {
protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

/** Test cases below */
TEST_F(MultibladeDataTest, Constructor) {
  DataParser mbdata;
  EXPECT_EQ(0, mbdata.Stats.error_bytes);
  EXPECT_EQ(nullptr, mbdata.MBData);
  EXPECT_EQ(nullptr, mbdata.MBHeader);
}


TEST_F(MultibladeDataTest, InvalidHeaderSizes) {
  DataParser mbdata;
  for (int i = 0; i < 32; i++) {
    auto res = mbdata.parse((char *)&all_zeroes[0], i);
    ASSERT_EQ(res, -mbdata.error::ESIZE);
    ASSERT_EQ(nullptr, mbdata.MBHeader);
    ASSERT_EQ(nullptr, mbdata.MBData);
    ASSERT_EQ(i, mbdata.Stats.error_bytes);
  }
  auto res = mbdata.parse((char *)&all_zeroes[0], 32);
  ASSERT_EQ(res, 0);
  ASSERT_NE(nullptr, mbdata.MBHeader);
  ASSERT_NE(nullptr, mbdata.MBData);
  ASSERT_EQ(0, mbdata.MBHeader->numElements);
}

TEST_F(MultibladeDataTest, Packet13Triggered) {
  unsigned int eventsInPacket{958};
  DataParser mbdata;
  auto res = mbdata.parse((char *)&pkt13[0], pkt13.size());

  EXPECT_EQ(res, eventsInPacket);
  EXPECT_EQ(0, mbdata.Stats.error_bytes);
  EXPECT_NE(nullptr, mbdata.MBData);
  EXPECT_NE(nullptr, mbdata.MBHeader);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
