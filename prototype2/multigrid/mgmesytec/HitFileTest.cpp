/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/HitFile.h>
#include <test/TestBase.h>

class MGHitFileTest : public TestBase {
protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(MGHitFileTest, CreateFile) {
  MGHitFile::create("readout_file_test.h5");
  EXPECT_TRUE(hdf5::file::is_hdf5_file("readout_file_test.h5"));
}

TEST_F(MGHitFileTest, OpenEmptyFile) {
  MGHitFile::create("readout_file_test.h5");
  auto file = MGHitFile::open("readout_file_test.h5");
  EXPECT_EQ(file.count(), 0);
}

TEST_F(MGHitFileTest, Write) {
  auto file = MGHitFile::create("readout_file_test.h5");
  file.data.resize(3);
  file.write();
  EXPECT_EQ(file.count(), 3);
  file.write();
  EXPECT_EQ(file.count(), 6);
  file.write();
  EXPECT_EQ(file.count(), 9);
}

TEST_F(MGHitFileTest, Read) {
  auto file_out = MGHitFile::create("readout_file_test.h5");
  file_out.data.resize(9);
  file_out.write();
  file_out = MGHitFile();

  auto file = MGHitFile::open("readout_file_test.h5");
  EXPECT_EQ(file.data.size(), 0);
  file.read_at(0, 3);
  EXPECT_EQ(file.data.size(), 3);
  file.read_at(0, 9);
  EXPECT_EQ(file.data.size(), 9);

  EXPECT_THROW(file.read_at(0, 12), std::runtime_error);
}

TEST_F(MGHitFileTest, ReadAll) {
  auto file_out = MGHitFile::create("readout_file_test.h5");
  file_out.data.resize(9);
  file_out.write();
  file_out = MGHitFile();

  std::vector<MGHit> data;
  MGHitFile::read("readout_file_test.h5", data);
  EXPECT_EQ(data.size(), 9);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
