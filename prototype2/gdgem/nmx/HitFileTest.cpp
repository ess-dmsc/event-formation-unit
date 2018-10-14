/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/HitFile.h>
#include <test/TestBase.h>

class NMXHitFileTest : public TestBase {
protected:
  virtual void SetUp() {
    hdf5::error::Singleton::instance().auto_print(false);
    if (boost::filesystem::exists("hit_file_test.h5"))
    {
      boost::filesystem::remove("hit_file_test.h5");
    }

    if (boost::filesystem::exists("hit_file_test_1.h5"))
    {
      boost::filesystem::remove("hit_file_test_1.h5");
    }
  }
  virtual void TearDown() {}
};

TEST_F(NMXHitFileTest, CreateFile) {
  HitFile::create("hit_file_test");
  EXPECT_TRUE(hdf5::file::is_hdf5_file("hit_file_test.h5"));
}

TEST_F(NMXHitFileTest, OpenEmptyFile) {
  HitFile::create("hit_file_test");
  auto file = HitFile::open("hit_file_test");
  EXPECT_EQ(file->count(), 0);
}

TEST_F(NMXHitFileTest, Push) {
  auto file = HitFile::create("hit_file_test");
  file->push(std::vector<Hit>(100, Hit()));
  EXPECT_EQ(file->count(), 0);
  file->push(std::vector<Hit>(900, Hit()));
  EXPECT_EQ(file->count(), 1000);
  file->push(std::vector<Hit>(3000, Hit()));
  EXPECT_EQ(file->count(), 4000);
}

TEST_F(NMXHitFileTest, PushFileRotation) {
  auto file = HitFile::create("hit_file_test", 1);
  file->push(std::vector<Hit>(100, Hit()));
  EXPECT_EQ(file->count(), 0);
  file->push(std::vector<Hit>(900, Hit()));
  EXPECT_EQ(file->count(), 1000);
  file->push(std::vector<Hit>(3000, Hit()));
  EXPECT_EQ(file->count(), 4000);

  EXPECT_TRUE(hdf5::file::is_hdf5_file("hit_file_test.h5"));

  EXPECT_FALSE(boost::filesystem::exists("hit_file_test_1.h5"));
  file->push(std::vector<Hit>(300000, Hit()));
  EXPECT_TRUE(hdf5::file::is_hdf5_file("hit_file_test_1.h5"));
}

TEST_F(NMXHitFileTest, Read) {
  auto file_out = HitFile::create("hit_file_test");
  file_out->push(std::vector<Hit>(900, Hit()));
  file_out.reset();

  auto file = HitFile::open("hit_file_test");
  EXPECT_EQ(file->Data.size(), 0);
  file->readAt(0, 3);
  EXPECT_EQ(file->Data.size(), 3);
  file->readAt(0, 9);
  EXPECT_EQ(file->Data.size(), 9);

  EXPECT_THROW(file->readAt(0, 1000), std::runtime_error);
}

TEST_F(NMXHitFileTest, ReadAll) {
  auto file_out = HitFile::create("hit_file_test");
  file_out->push(std::vector<Hit>(900, Hit()));
  file_out.reset();

  std::vector<Hit> data;
  HitFile::read("hit_file_test", data);
  EXPECT_EQ(data.size(), 900);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
