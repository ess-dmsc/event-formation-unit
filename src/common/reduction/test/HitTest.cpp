// Copyright (C) 2016, 2017 European Spallation Source ERIC

#include <common/reduction/Hit.h>
#include <common/testutils/TestBase.h>

class HitTest : public TestBase {
protected:
  Hit hit;
  void SetUp() override {
    hdf5::error::Singleton::instance().auto_print(false);
    if (std::filesystem::exists("hit_file_test_00000.h5")) {
      std::filesystem::remove("hit_file_test_00000.h5");
    }
  }
  void TearDown() override {}
};

TEST_F(HitTest, Debug) { EXPECT_FALSE(hit.to_string().empty()); }

TEST_F(HitTest, CompoundMapping) {
  // If you are forced to change anything here,
  // you have broken dumpfile compatibility, and you should
  // bump FormatVersion for the struct

  auto t = hdf5::datatype::create<Hit>();

  EXPECT_EQ(t.number_of_fields(), 4ul);
  EXPECT_EQ(t.get_class(), hdf5::datatype::Class::Compound);

  auto ct = hdf5::datatype::Compound(t);

  EXPECT_EQ(ct.field_name(0), "time");
  EXPECT_EQ(ct[0], hdf5::datatype::create<uint64_t>());

  EXPECT_EQ(ct.field_name(1), "coordinate");
  EXPECT_EQ(ct[1], hdf5::datatype::create<uint16_t>());

  EXPECT_EQ(ct.field_name(2), "weight");
  EXPECT_EQ(ct[2], hdf5::datatype::create<uint16_t>());

  EXPECT_EQ(ct.field_name(3), "plane");
  EXPECT_EQ(ct[3], hdf5::datatype::create<uint8_t>());
}

TEST_F(HitTest, CreateFile) {
  HitFile::create("hit_file_test");
  EXPECT_TRUE(hdf5::file::is_hdf5_file("hit_file_test.h5"));
  HitFile::create("hit_file_test", 10000);
  EXPECT_TRUE(hdf5::file::is_hdf5_file("hit_file_test_00000.h5"));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
