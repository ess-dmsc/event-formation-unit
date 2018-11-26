/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/DumpFile.h>
#include <test/TestBase.h>

struct __attribute__ ((packed)) Hit {
  size_t a{0};
  int8_t b{0};
  uint32_t c{0};

  static std::string DatasetName() { return "hit_dataset"; }
  static uint16_t FormatVersion() { return 0; }
};

namespace hdf5 {
namespace datatype {
template<>
class TypeTrait<Hit> {
public:
  H5_COMPOUND_DEFINE_TYPE(Hit) {
    H5_COMPOUND_INIT;
    H5_COMPOUND_INSERT_MEMBER(a);
    H5_COMPOUND_INSERT_MEMBER(b);
    H5_COMPOUND_INSERT_MEMBER(c);
    H5_COMPOUND_RETURN;
  }
};
}
}

using HitFile = DumpFile<Hit>;

struct __attribute__ ((packed)) Hit2 {
  size_t a{0};
  int8_t b{0};
  uint32_t c{0};

  static std::string DatasetName() { return "hit_dataset"; }
  static uint16_t FormatVersion() { return 1; }
};

namespace hdf5 {
namespace datatype {
template<>
class TypeTrait<Hit2> {
public:
  H5_COMPOUND_DEFINE_TYPE(Hit2) {
    H5_COMPOUND_INIT;
    H5_COMPOUND_INSERT_MEMBER(a);
    H5_COMPOUND_INSERT_MEMBER(b);
    H5_COMPOUND_INSERT_MEMBER(c);
    H5_COMPOUND_RETURN;
  }
};
}
}

using Hit2File = DumpFile<Hit2>;

class DumpFileTest : public TestBase {
protected:
  virtual void SetUp() {
    hdf5::error::Singleton::instance().auto_print(false);
    if (boost::filesystem::exists("dumpfile_test_00000.h5"))
    {
      boost::filesystem::remove("dumpfile_test_00000.h5");
    }

    if (boost::filesystem::exists("dumpfile_test_00001.h5"))
    {
      boost::filesystem::remove("dumpfile_test_00001.h5");
    }
  }
  virtual void TearDown() {}
};

TEST_F(DumpFileTest, CreateFile) {
  HitFile::create("dumpfile_test");
  EXPECT_TRUE(hdf5::file::is_hdf5_file("dumpfile_test_00000.h5"));
}

TEST_F(DumpFileTest, OpenEmptyFile) {
  HitFile::create("dumpfile_test");
  auto file = HitFile::open("dumpfile_test_00000");
  EXPECT_EQ(file->count(), 0);
}

TEST_F(DumpFileTest, PushOne) {
  auto file = HitFile::create("dumpfile_test");
  file->push(Hit());
  EXPECT_EQ(file->count(), 0);
  for (size_t i=0; i < 1000; ++i)
    file->push(Hit());
  EXPECT_EQ(file->count(), 9000 / sizeof(Hit));
}

TEST_F(DumpFileTest, Push) {
  auto file = HitFile::create("dumpfile_test");
  file->push(std::vector<Hit>(100, Hit()));
  EXPECT_EQ(file->count(), 0);
  file->push(std::vector<Hit>(900, Hit()));
  EXPECT_EQ(file->count(), 1000);
  file->push(std::vector<Hit>(3000, Hit()));
  EXPECT_EQ(file->count(), 4000);
}

TEST_F(DumpFileTest, WrongVersion) {
  auto file = HitFile::create("dumpfile_test");
  file->push(std::vector<Hit>(1000, Hit()));
  EXPECT_EQ(file->count(), 1000);
  file.reset();

  EXPECT_ANY_THROW(Hit2File::open("dumpfile_test_00000"));
}

TEST_F(DumpFileTest, PushFileRotation) {
  auto file = HitFile::create("dumpfile_test", 1);
  file->push(std::vector<Hit>(100, Hit()));
  EXPECT_EQ(file->count(), 0);
  file->push(std::vector<Hit>(900, Hit()));
  EXPECT_EQ(file->count(), 1000);
  file->push(std::vector<Hit>(3000, Hit()));
  EXPECT_EQ(file->count(), 4000);

  EXPECT_TRUE(hdf5::file::is_hdf5_file("dumpfile_test_00000.h5"));

  EXPECT_FALSE(boost::filesystem::exists("dumpfile_test_00001.h5"));
  file->push(std::vector<Hit>(300000, Hit()));
  EXPECT_TRUE(hdf5::file::is_hdf5_file("dumpfile_test_00001.h5"));
}

TEST_F(DumpFileTest, Read) {
  auto file_out = HitFile::create("dumpfile_test");
  file_out->push(std::vector<Hit>(900, Hit()));
  file_out.reset();

  auto file = HitFile::open("dumpfile_test_00000");
  EXPECT_EQ(file->Data.size(), 0);
  file->readAt(0, 3);
  EXPECT_EQ(file->Data.size(), 3);
  file->readAt(0, 9);
  EXPECT_EQ(file->Data.size(), 9);

  EXPECT_THROW(file->readAt(0, 1000), std::runtime_error);
}

TEST_F(DumpFileTest, ReadAll) {
  auto file_out = HitFile::create("dumpfile_test");
  file_out->push(std::vector<Hit>(900, Hit()));
  file_out.reset();

  std::vector<Hit> data;
  HitFile::read("dumpfile_test_00000", data);
  EXPECT_EQ(data.size(), 900);
}

TEST_F(DumpFileTest, FlushOnClose) {
  auto file_out = HitFile::create("dumpfile_test");
  file_out->push(std::vector<Hit>(10, Hit()));
  EXPECT_EQ(file_out->count(), 0);
  file_out.reset();

  std::vector<Hit> data;
  HitFile::read("dumpfile_test_00000", data);
  EXPECT_EQ(data.size(), 10);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
