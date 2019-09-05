/** Copyright (C) 2017-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <multiblade/caen/DataParser.h>
#include <multiblade/caen/DataParserTestData.h>
#include <test/TestBase.h>

class MultibladeDataTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

/** Test cases below */
TEST_F(MultibladeDataTest, Constructor) {
  Multiblade::DataParser mbdata;
  EXPECT_EQ(0, mbdata.Stats.error_bytes);
  EXPECT_EQ(nullptr, mbdata.Data);
  EXPECT_EQ(nullptr, mbdata.MBHeader);
}


TEST_F(MultibladeDataTest, InvalidHeaderSizes) {
  Multiblade::DataParser mbdata;
  for (int i = 0; i < 32; i++) {
    auto res = mbdata.parse((char *)&all_zeroes[0], i);
    ASSERT_EQ(res, -mbdata.error::ESIZE);
    ASSERT_EQ(nullptr, mbdata.MBHeader);
    ASSERT_EQ(nullptr, mbdata.Data);
    ASSERT_EQ(i, mbdata.Stats.error_bytes);
  }
  auto res = mbdata.parse((char *)&all_zeroes[0], 32);
  ASSERT_EQ(res, 0);
  ASSERT_NE(nullptr, mbdata.MBHeader);
  ASSERT_NE(nullptr, mbdata.Data);
  ASSERT_EQ(0, mbdata.MBHeader->numElements);
}


TEST_F(MultibladeDataTest, DataLengthMisMatch) {
  Multiblade::DataParser mbdata;
  auto res = mbdata.parse((char *)&all_zeroes[0], all_zeroes.size());
  ASSERT_EQ(res, -Multiblade::DataParser::error::ESIZE);
}

TEST_F(MultibladeDataTest, InvalidVersion) {
  Multiblade::DataParser mbdata;
  auto res = mbdata.parse((char *)&invalid_version[0], invalid_version.size());
  ASSERT_EQ(res, -Multiblade::DataParser::error::EHEADER);
}

TEST_F(MultibladeDataTest, InvalidType) {
  Multiblade::DataParser mbdata;
  auto res = mbdata.parse((char *)&invalid_type[0], invalid_type.size());
  ASSERT_EQ(res, -Multiblade::DataParser::error::EHEADER);
}

TEST_F(MultibladeDataTest, Packet13Triggered) {
  unsigned int eventsInPacket{958};
  Multiblade::DataParser mbdata;
  auto res = mbdata.parse((char *)&pkt13[0], pkt13.size());

  EXPECT_EQ(res, eventsInPacket);
  EXPECT_EQ(0, mbdata.Stats.error_bytes);
  EXPECT_NE(nullptr, mbdata.Data);
  EXPECT_NE(nullptr, mbdata.MBHeader);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
