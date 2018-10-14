/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/ReadoutFile.h>
#include <test/TestBase.h>

class ReadoutFileTest : public TestBase {
protected:
  virtual void SetUp() {
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

TEST_F(ReadoutFileTest, CreateFile) {
  ReadoutFile::create("readout_file_test");
  EXPECT_TRUE(hdf5::file::is_hdf5_file("readout_file_test.h5"));
}

TEST_F(ReadoutFileTest, OpenEmptyFile) {
  ReadoutFile::create("readout_file_test");
  auto file = ReadoutFile::open("readout_file_test");
  EXPECT_EQ(file->count(), 0);
}

TEST_F(ReadoutFileTest, Push) {
  auto file = ReadoutFile::create("readout_file_test");
  file->push(std::vector<Readout>(100, Readout()));
  EXPECT_EQ(file->count(), 0);
  file->push(std::vector<Readout>(900, Readout()));
  EXPECT_EQ(file->count(), 1000);
  file->push(std::vector<Readout>(3000, Readout()));
  EXPECT_EQ(file->count(), 4000);
}

TEST_F(ReadoutFileTest, PushFileRotation) {
  auto file = ReadoutFile::create("readout_file_test", 1);
  file->push(std::vector<Readout>(100, Readout()));
  EXPECT_EQ(file->count(), 0);
  file->push(std::vector<Readout>(900, Readout()));
  EXPECT_EQ(file->count(), 1000);
  file->push(std::vector<Readout>(3000, Readout()));
  EXPECT_EQ(file->count(), 4000);

  EXPECT_TRUE(hdf5::file::is_hdf5_file("readout_file_test.h5"));

  EXPECT_FALSE(boost::filesystem::exists("readout_file_test_1.h5"));
  file->push(std::vector<Readout>(300000, Readout()));
  EXPECT_TRUE(hdf5::file::is_hdf5_file("readout_file_test_1.h5"));
}

TEST_F(ReadoutFileTest, Read) {
  auto file_out = ReadoutFile::create("readout_file_test");
  file_out->push(std::vector<Readout>(900, Readout()));
  file_out.reset();

  auto file = ReadoutFile::open("readout_file_test");
  EXPECT_EQ(file->Data.size(), 0);
  file->readAt(0, 3);
  EXPECT_EQ(file->Data.size(), 3);
  file->readAt(0, 9);
  EXPECT_EQ(file->Data.size(), 9);

  EXPECT_THROW(file->readAt(0, 1000), std::runtime_error);
}

TEST_F(ReadoutFileTest, ReadAll) {
  auto file_out = ReadoutFile::create("readout_file_test");
  file_out->push(std::vector<Readout>(900, Readout()));
  file_out.reset();

  std::vector<Readout> data;
  ReadoutFile::read("readout_file_test", data);
  EXPECT_EQ(data.size(), 900);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
