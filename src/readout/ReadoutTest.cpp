/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <readout/Readout.h>
#include <readout/ReadoutTestData.h>
#include <test/TestBase.h>

class ReadoutTest : public TestBase {
protected:
  Readout RdOut;
  const int DataType{0x30};
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ReadoutTest, Constructor) {
  ASSERT_EQ(RdOut.Stats.ErrorBuffer, 0);
  ASSERT_EQ(RdOut.Stats.ErrorSize, 0);
  ASSERT_EQ(RdOut.Stats.ErrorVersion, 0);
  ASSERT_EQ(RdOut.Stats.ErrorTypeSubType, 0);
  ASSERT_EQ(RdOut.Stats.ErrorSeqNum, 0);
}

// nullptr as buffer
TEST_F(ReadoutTest, ErrorBuffer) {
  auto Res = RdOut.validate(0, 100, DataType);
  ASSERT_EQ(Res, -Readout::EBUFFER);
  ASSERT_EQ(RdOut.Stats.ErrorBuffer, 1);
}

TEST_F(ReadoutTest, HeaderLTMin) {
  auto Res = RdOut.validate((char *)&ErrCookie[0], 3, Readout::Loki4Amp);
  ASSERT_EQ(Res, -Readout::ESIZE);
  ASSERT_EQ(RdOut.Stats.ErrorSize, 1);
}

TEST_F(ReadoutTest, HeaderGTMax) {
  auto Res = RdOut.validate((char *)&ErrCookie[0], 8973, DataType);
  ASSERT_EQ(Res, -Readout::ESIZE);
  ASSERT_EQ(RdOut.Stats.ErrorSize, 1);
}

TEST_F(ReadoutTest, ErrorCookie) {
  auto Res = RdOut.validate((char *)&ErrCookie[0], ErrCookie.size(), DataType);
  ASSERT_EQ(Res, -Readout::EHEADER);
  ASSERT_EQ(RdOut.Stats.ErrorVersion, 1);
}

TEST_F(ReadoutTest, ErrorVersion) {
  auto Res = RdOut.validate((char *)&ErrVersion[0], ErrVersion.size(), DataType);
  ASSERT_EQ(Res, -Readout::EHEADER);
  ASSERT_EQ(RdOut.Stats.ErrorVersion, 1);
}

TEST_F(ReadoutTest, OkVersion) {
  unsigned int Errors{0};
  for (unsigned int Size = 4; Size < OkVersion.size(); Size++) {
    Errors++;
    auto Res = RdOut.validate((char *)&OkVersion[0], Size, DataType);
    ASSERT_EQ(Res, -Readout::ESIZE);
    ASSERT_EQ(RdOut.Stats.ErrorSize, Errors);
    ASSERT_EQ(RdOut.Packet.DataPtr, nullptr);
  }
  auto Res = RdOut.validate((char *)&OkVersion[0], OkVersion.size(), DataType);
  ASSERT_EQ(Res, Readout::OK);
  ASSERT_EQ(RdOut.Packet.DataPtr, (char *)(&OkVersion[0] + sizeof(Readout::PacketHeaderV0)));
  ASSERT_EQ(RdOut.Packet.DataLength, 0);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
