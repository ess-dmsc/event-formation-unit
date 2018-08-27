/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/HitFile.h>
#include <test/TestBase.h>

using namespace Multigrid;

class HitFileTest : public TestBase {
protected:
  virtual void SetUp() {
    hdf5::error::Singleton::instance().auto_print(false);
    if (boost::filesystem::exists("readout_file_test.h5"))
    {
      boost::filesystem::remove("readout_file_test.h5");
    }

    if (boost::filesystem::exists("readout_file_test_1.h5"))
    {
      boost::filesystem::remove("readout_file_test_1.h5");
    }
  }
  virtual void TearDown() {}
};

TEST_F(HitFileTest, CreateFile) {
  HitFile::create("readout_file_test");
  EXPECT_TRUE(hdf5::file::is_hdf5_file("readout_file_test.h5"));
}

TEST_F(HitFileTest, OpenEmptyFile) {
  HitFile::create("readout_file_test");
  auto file = HitFile::open("readout_file_test");
  EXPECT_EQ(file->count(), 0);
}

TEST_F(HitFileTest, Push) {
  auto file = HitFile::create("readout_file_test");
  file->push(std::vector<Hit>(100, Hit()));
  EXPECT_EQ(file->count(), 0);
  file->push(std::vector<Hit>(900, Hit()));
  EXPECT_EQ(file->count(), 1000);
  file->push(std::vector<Hit>(3000, Hit()));
  EXPECT_EQ(file->count(), 4000);
}

TEST_F(HitFileTest, PushFileRotation) {
  auto file = HitFile::create("readout_file_test", 1);
  file->push(std::vector<Hit>(100, Hit()));
  EXPECT_EQ(file->count(), 0);
  file->push(std::vector<Hit>(900, Hit()));
  EXPECT_EQ(file->count(), 1000);
  file->push(std::vector<Hit>(3000, Hit()));
  EXPECT_EQ(file->count(), 4000);

  EXPECT_TRUE(hdf5::file::is_hdf5_file("readout_file_test.h5"));

  EXPECT_FALSE(boost::filesystem::exists("readout_file_test_1.h5"));
  file->push(std::vector<Hit>(300000, Hit()));
  EXPECT_TRUE(hdf5::file::is_hdf5_file("readout_file_test_1.h5"));
}

TEST_F(HitFileTest, Read) {
  auto file_out = HitFile::create("readout_file_test");
  file_out->push(std::vector<Hit>(900, Hit()));
  file_out.reset();

  auto file = HitFile::open("readout_file_test");
  EXPECT_EQ(file->data.size(), 0);
  file->read_at(0, 3);
  EXPECT_EQ(file->data.size(), 3);
  file->read_at(0, 9);
  EXPECT_EQ(file->data.size(), 9);

  EXPECT_THROW(file->read_at(0, 1000), std::runtime_error);
}

TEST_F(HitFileTest, ReadAll) {
  auto file_out = HitFile::create("readout_file_test");
  file_out->push(std::vector<Hit>(900, Hit()));
  file_out.reset();

  std::vector<Hit> data;
  HitFile::read("readout_file_test", data);
  EXPECT_EQ(data.size(), 900);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
