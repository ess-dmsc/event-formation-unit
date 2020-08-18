/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#define H5_USE_NEW_COMPOUND_IMPL 1
#include <common/DumpFile.h>
#undef H5_USE_NEW_COMPOUND_IMPL
#include <test/TestBase.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <h5cpp/hdf5.hpp>
#pragma GCC diagnostic pop

#include <limits>

struct __attribute__ ((packed)) PrimHit {
  static const char *DatasetName() { return "hit_dataset"; }
  static uint16_t FormatVersion() { return 0; }

  /// !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  uint64_t time{0};
  uint16_t coordinate{0};
  uint16_t weight{0};
  // \todo uint8 might not be enough, if detectors have more independent modules/segments
  uint8_t plane{0};
  /// !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  /// \brief prints values for debug purposes
  std::string to_string() const;

  static constexpr uint16_t InvalidCoord {std::numeric_limits<uint16_t>::max()};
  static constexpr uint8_t InvalidPlane {std::numeric_limits<uint8_t>::max()};
  static constexpr uint8_t PulsePlane {std::numeric_limits<uint8_t>::max() - 1};
};

struct __attribute__ ((packed)) PrimHit2 {
  static const char *DatasetName() { return "hit_dataset"; }
  static uint16_t FormatVersion() { return 1; }

  /// !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  uint64_t time{0};
  uint16_t coordinate{0};
  uint16_t weight{0};
  // \todo uint8 might not be enough, if detectors have more independent modules/segments
  uint8_t plane{0};
  /// !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  /// \brief prints values for debug purposes
  std::string to_string() const;

  static constexpr uint16_t InvalidCoord {std::numeric_limits<uint16_t>::max()};
  static constexpr uint8_t InvalidPlane {std::numeric_limits<uint8_t>::max()};
  static constexpr uint8_t PulsePlane {std::numeric_limits<uint8_t>::max() - 1};
};

H5_PRIM_COMPOUND_BEGIN(PrimHit)
H5_PRIM_COMPOUND_MEMBER(time)
H5_PRIM_COMPOUND_MEMBER(coordinate)
H5_PRIM_COMPOUND_MEMBER(weight)
H5_PRIM_COMPOUND_MEMBER(plane)
H5_PRIM_COMPOUND_END()

H5_PRIM_COMPOUND_BEGIN(PrimHit2)
H5_PRIM_COMPOUND_MEMBER(time)
H5_PRIM_COMPOUND_MEMBER(coordinate)
H5_PRIM_COMPOUND_MEMBER(weight)
H5_PRIM_COMPOUND_MEMBER(plane)
H5_PRIM_COMPOUND_END()

using PrimHitFile = PrimDumpFile<PrimHit>;
using PrimHitFile2 = PrimDumpFile<PrimHit2>;

class DumpPrimFileTest : public TestBase {
  void SetUp() override {
    hdf5::error::Singleton::instance().auto_print(false);
    if (boost::filesystem::exists("dumpfile_test_00000.h5")) {
      boost::filesystem::remove("dumpfile_test_00000.h5");
    }

    if (boost::filesystem::exists("dumpfile_test_00001.h5")) {
      boost::filesystem::remove("dumpfile_test_00001.h5");
    }
  }
  void TearDown() override {}
};

TEST_F(DumpPrimFileTest, CreateFile) {
  PrimHitFile::create("dumpfile_test");
  EXPECT_TRUE(hdf5::file::is_hdf5_file("dumpfile_test_00000.h5"));
}

TEST_F(DumpPrimFileTest, OpenEmptyFile) {
  PrimHitFile::create("dumpfile_test");
  auto file = PrimHitFile::open("dumpfile_test_00000");
  EXPECT_EQ(file->count(), 0);
}

TEST_F(DumpPrimFileTest, PushOne) {
  auto file = PrimHitFile::create("dumpfile_test");
  file->push(PrimHit());
  EXPECT_EQ(file->count(), 0);
  for (size_t i = 0; i < 1000; ++i)
    file->push(PrimHit());
  EXPECT_EQ(file->count(), 9000 / sizeof(PrimHit));
}

TEST_F(DumpPrimFileTest, Push) {
  auto file = PrimHitFile::create("dumpfile_test");
  file->push(std::vector<PrimHit>(100, PrimHit()));
  EXPECT_EQ(file->count(), 0);
  file->push(std::vector<PrimHit>(900, PrimHit()));
  EXPECT_EQ(file->count(), 1000);
  file->push(std::vector<PrimHit>(3000, PrimHit()));
  EXPECT_EQ(file->count(), 4000);
}

TEST_F(DumpPrimFileTest, WrongVersion) {
  auto file = PrimHitFile::create("dumpfile_test");
  file->push(std::vector<PrimHit>(1000, PrimHit()));
  EXPECT_EQ(file->count(), 1000);
  file.reset();

  EXPECT_ANY_THROW(PrimHitFile2::open("dumpfile_test_00000"));
}

TEST_F(DumpPrimFileTest, PushFileRotation) {
  auto file = PrimHitFile::create("dumpfile_test", 1);
  file->push(std::vector<PrimHit>(100, PrimHit()));
  EXPECT_EQ(file->count(), 0);
  file->push(std::vector<PrimHit>(900, PrimHit()));
  EXPECT_EQ(file->count(), 1000);
  file->push(std::vector<PrimHit>(3000, PrimHit()));
  EXPECT_EQ(file->count(), 4000);

  EXPECT_TRUE(hdf5::file::is_hdf5_file("dumpfile_test_00000.h5"));

  EXPECT_FALSE(boost::filesystem::exists("dumpfile_test_00001.h5"));
  file->push(std::vector<PrimHit>(300000, PrimHit()));
  EXPECT_TRUE(hdf5::file::is_hdf5_file("dumpfile_test_00001.h5"));
}

// clang-format off
// This gives a validation error when hdf5::error::Singleton::instance().auto_print(false) is disabled:
//   HDF5-DIAG: Error detected in HDF5 (1.10.5) thread 0:
//     #000: ../source_subfolder/src/H5Dio.c line 185 in H5Dread(): could not get a validated dataspace from file_space_id
//       major: Invalid arguments to routine
//       minor: Bad value
//     #001: ../source_subfolder/src/H5S.c line 254 in H5S_get_validated_dataspace(): selection + offset not within extent
//       major: Dataspace
//       minor: Out of range
// clang-format on
TEST_F(DumpPrimFileTest, Read) {
  auto file_out = PrimHitFile::create("dumpfile_test");
  file_out->push(std::vector<PrimHit>(900, PrimHit()));
  file_out.reset();

  auto file = PrimHitFile::open("dumpfile_test_00000");
  EXPECT_EQ(file->Data.size(), 0);
  file->readAt(0, 3);
  EXPECT_EQ(file->Data.size(), 3);
  file->readAt(0, 9);
  EXPECT_EQ(file->Data.size(), 9);

  EXPECT_THROW(file->readAt(0, 1000), std::runtime_error);
}

TEST_F(DumpPrimFileTest, ReadAll) {
  auto file_out = PrimHitFile::create("dumpfile_test");
  file_out->push(std::vector<PrimHit>(900, PrimHit()));
  file_out.reset();

  std::vector<PrimHit> data;
  PrimHitFile::read("dumpfile_test_00000", data);
  EXPECT_EQ(data.size(), 900);
}

TEST_F(DumpPrimFileTest, FlushOnClose) {
  auto file_out = PrimHitFile::create("dumpfile_test");
  file_out->push(std::vector<PrimHit>(10, PrimHit()));
  EXPECT_EQ(file_out->count(), 0);
  file_out.reset();

  std::vector<PrimHit> data;
  PrimHitFile::read("dumpfile_test_00000", data);
  EXPECT_EQ(data.size(), 10);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define H5_USE_NEW_COMPOUND_IMPL 0
#include <common/DumpFile.h>
#undef H5_USE_NEW_COMPOUND_IMPL

struct __attribute__ ((packed)) Hit {
  size_t a{0};
  int8_t b{0};
  uint32_t c{0};

  static const char *DatasetName() { return "hit_dataset"; }
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

  static const char *DatasetName() { return "hit_dataset"; }
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
  void SetUp() override {
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
  void TearDown() override {}
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

// clang-format off
// This gives a validation error when hdf5::error::Singleton::instance().auto_print(false) is disabled:
//   HDF5-DIAG: Error detected in HDF5 (1.10.5) thread 0:
//     #000: ../source_subfolder/src/H5Dio.c line 185 in H5Dread(): could not get a validated dataspace from file_space_id
//       major: Invalid arguments to routine
//       minor: Bad value
//     #001: ../source_subfolder/src/H5S.c line 254 in H5S_get_validated_dataspace(): selection + offset not within extent
//       major: Dataspace
//       minor: Out of range
// clang-format on
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
