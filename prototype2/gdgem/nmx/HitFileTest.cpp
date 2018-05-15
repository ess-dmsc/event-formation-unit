/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/HitFile.h>
#include <test/TestBase.h>

class HitFileTest : public TestBase {
protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(HitFileTest, CreateFile) {
  HitFile::create("readout_file_test.h5");
  EXPECT_TRUE(hdf5::file::is_hdf5_file("readout_file_test.h5"));
}

TEST_F(HitFileTest, OpenEmptyFile) {
  HitFile::create("readout_file_test.h5");
  auto file = HitFile::open("readout_file_test.h5");
  EXPECT_EQ(file.count(), 0);
}

TEST_F(HitFileTest, Write) {
  auto file = HitFile::create("readout_file_test.h5");
  file.data.resize(3);
  file.write();
  EXPECT_EQ(file.count(), 3);
  file.write();
  EXPECT_EQ(file.count(), 6);
  file.write();
  EXPECT_EQ(file.count(), 9);
}

TEST_F(HitFileTest, Read) {
  auto file_out = HitFile::create("readout_file_test.h5");
  file_out.data.resize(9);
  file_out.write();
  file_out = HitFile();

  auto file = HitFile::open("readout_file_test.h5");
  EXPECT_EQ(file.data.size(), 0);
  file.read_at(0, 3);
  EXPECT_EQ(file.data.size(), 3);
  file.read_at(0, 9);
  EXPECT_EQ(file.data.size(), 9);

  EXPECT_THROW(file.read_at(0, 12), std::runtime_error);
}

TEST_F(HitFileTest, ReadAll) {
  auto file_out = HitFile::create("readout_file_test.h5");
  file_out.data.resize(9);
  file_out.write();
  file_out = HitFile();

  std::vector<Hit> data;
  HitFile::read("readout_file_test.h5", data);
  EXPECT_EQ(data.size(), 9);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
