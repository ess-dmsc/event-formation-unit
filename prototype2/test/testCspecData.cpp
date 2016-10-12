/** Copyright (C) 2016 European Spallation Source */

#include <CSPECData.h>
#include <gtest/gtest.h>

using namespace std;


std::vector<unsigned int> ok_one{0x40010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
                         0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd};

std::vector<unsigned int> ok_two{0x40010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
                         0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
                         0x40010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
                         0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd};

std::vector<std::vector<unsigned int>> ok{ok_one, ok_two};


std::vector<unsigned int> err_hdr{0x30010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
                          0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd};

std::vector<unsigned int> err_hdr2{0x40010008, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
                         0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd};

std::vector<unsigned int> err_dat{0x40010009, 0x0400020f, 0x8401002f, 0x0402028f, 0x0403002b,
                          0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd};

std::vector<unsigned int> err_ftr{0x40010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
                          0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0x400211dd};

std::vector<std::vector<unsigned int>> err_pkt{err_hdr, err_hdr2, err_dat, err_ftr};


std::vector<unsigned int> err_short{0x40010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
                            0x040402d3, 0x04050035, 0x04060385, 0x0407002c};

std::vector<unsigned int> err_long{0x40010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
                           0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
                           0x40010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
                           0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
                           0x40010009};

std::vector<std::vector<unsigned int>> err_size{err_short, err_long};

/** Test fixture and tests below */

class CspecDataTest : public ::testing::Test {
};

TEST_F(CspecDataTest, ValidData) {
  for (auto v : ok) {
    auto dat = new CSPECData((char *)&v[0], v.size()*4);
    ASSERT_EQ(dat->ierror, 0);
    ASSERT_EQ(dat->idata, v.size()*4/40);
    ASSERT_EQ(dat->dataq.size(), v.size()*4/40);
    ASSERT_EQ(dat->data.module, 1);
  }
}

TEST_F(CspecDataTest, InvalidData) {
  for (auto v : err_pkt) {
    auto dat = new CSPECData((char *)&v[0], v.size()*4);
    ASSERT_EQ(dat->ierror, 1);
    ASSERT_EQ(dat->idata, 0);
  }
}

TEST_F(CspecDataTest, OverUndersizeData) {
for (auto v : err_size) {
  auto dat = new CSPECData((char *)&v[0], v.size()*4);
  ASSERT_EQ(dat->ierror, 0);
  ASSERT_EQ(dat->idata, v.size()*4/40);
  ASSERT_EQ(dat->ifrag, 1);
  }
}

TEST_F(CspecDataTest, ValidateGenerator) {
  char buffer[9000];
  int size;
  for (int i = 1; i < 225; i++) {
    size = CSPECData::generate(buffer, 9000, i);
    CSPECData dat(buffer, size);
    ASSERT_EQ(dat.idata, i);
    ASSERT_EQ(dat.ifrag, 0);
    ASSERT_EQ(dat.ierror, 0);
  }

  /** Test for oversize specification */
  size = CSPECData::generate(buffer, 9000, 300);
  CSPECData dat(buffer, size);
  ASSERT_EQ(dat.idata, 225);
  ASSERT_EQ(dat.ifrag, 0);
  ASSERT_EQ(dat.ierror, 0);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
