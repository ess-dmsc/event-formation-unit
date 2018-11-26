/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Hit.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

using namespace Gem;

class NMXHitTest : public TestBase {
protected:
  Hit hit;
  virtual void SetUp() {
    hdf5::error::Singleton::instance().auto_print(false);
    if (boost::filesystem::exists("hit_file_test_00000.h5"))
    {
      boost::filesystem::remove("hit_file_test_00000.h5");
    }
  }
  virtual void TearDown() { }
};

TEST_F(NMXHitTest, Debug) {
  ASSERT_FALSE(hit.debug().empty());
}

TEST_F(NMXHitTest, CompoundMapping) {
  // If you are forced to change anything here,
  // you have broken dumpfile compatibility, and you should
  // bump FormatVersion for the struct

  auto t = hdf5::datatype::create<Hit>();

  EXPECT_EQ(t.number_of_fields(), 4ul);
  EXPECT_EQ(t.get_class(), hdf5::datatype::Class::COMPOUND);

  auto ct = hdf5::datatype::Compound(t);

  EXPECT_EQ(ct.field_name(0), "time");
  EXPECT_EQ(ct[0], hdf5::datatype::create<double>());

  EXPECT_EQ(ct.field_name(1), "plane_id");
  EXPECT_EQ(ct[1], hdf5::datatype::create<uint8_t>());

  EXPECT_EQ(ct.field_name(2), "strip");
  EXPECT_EQ(ct[2], hdf5::datatype::create<uint16_t>());

  EXPECT_EQ(ct.field_name(3), "adc");
  EXPECT_EQ(ct[3], hdf5::datatype::create<uint16_t>());
}

TEST_F(NMXHitTest, CreateFile) {
  HitFile::create("hit_file_test");
  EXPECT_TRUE(hdf5::file::is_hdf5_file("hit_file_test_00000.h5"));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
