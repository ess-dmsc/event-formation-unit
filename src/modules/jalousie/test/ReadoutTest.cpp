/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <jalousie/Readout.h>
#include <common/testutils/TestBase.h>

using namespace Jalousie;

class JalSumoMappingsTest : public TestBase {
protected:
  void SetUp() override {
    hdf5::error::Singleton::instance().auto_print(false);
    if (boost::filesystem::exists("hit_file_test_00000.h5"))
    {
      boost::filesystem::remove("hit_file_test_00000.h5");
    }
  }
  void TearDown() override {}
};

TEST_F(JalSumoMappingsTest, PrintsSelf) {
  Readout h;
  EXPECT_FALSE(h.debug().empty());
  // Don't really care about particular contents here
}

TEST_F(JalSumoMappingsTest, CompoundMapping) {
  // If you are forced to change anything here,
  // you have broken dumpfile compatibility, and you should
  // bump FormatVersion for the struct

  auto t = hdf5::datatype::create<Readout>();

  EXPECT_EQ(t.number_of_fields(), 5ul);
  EXPECT_EQ(t.get_class(), hdf5::datatype::Class::COMPOUND);

  auto ct = hdf5::datatype::Compound(t);

  EXPECT_EQ(ct.field_name(0), "board");
  EXPECT_EQ(ct[0], hdf5::datatype::create<uint32_t>());

  EXPECT_EQ(ct.field_name(1), "sub_id");
  EXPECT_EQ(ct[1], hdf5::datatype::create<uint8_t>());

  EXPECT_EQ(ct.field_name(2), "time");
  EXPECT_EQ(ct[2], hdf5::datatype::create<uint64_t>());

  EXPECT_EQ(ct.field_name(3), "anode");
  EXPECT_EQ(ct[3], hdf5::datatype::create<uint8_t>());

  EXPECT_EQ(ct.field_name(4), "cathode");
  EXPECT_EQ(ct[4], hdf5::datatype::create<uint8_t>());
}

TEST_F(JalSumoMappingsTest, CreateFile) {
  ReadoutFile::create("hit_file_test");
  EXPECT_TRUE(hdf5::file::is_hdf5_file("hit_file_test_00000.h5"));
}

// \todo write random data and read it, confirm that it is the same

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
