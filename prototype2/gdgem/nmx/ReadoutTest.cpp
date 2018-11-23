/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Readout.h>
#include <test/TestBase.h>

using namespace Gem;

class NMXReadoutTest : public TestBase {
protected:
  virtual void SetUp() {
    if (boost::filesystem::exists("readout_file_test_00000.h5"))
    {
      boost::filesystem::remove("readout_file_test_00000.h5");
    }
  }
  virtual void TearDown() {}
};

TEST_F(NMXReadoutTest, CompoundMapping) {
  // If you are forced to change anything here,
  // you have broken dumpfile compatibility, and you should
  // bump FormatVersion for the struct

  auto t = hdf5::datatype::create<Readout>();

  EXPECT_EQ(t.number_of_fields(), 9ul);
  EXPECT_EQ(t.get_class(), hdf5::datatype::Class::COMPOUND);

  auto ct = hdf5::datatype::Compound(t);

  EXPECT_EQ(ct.field_name(0), "fec");
  EXPECT_EQ(ct[0], hdf5::datatype::create<uint8_t >());

  EXPECT_EQ(ct.field_name(1), "chip_id");
  EXPECT_EQ(ct[1], hdf5::datatype::create<uint8_t>());

  EXPECT_EQ(ct.field_name(2), "srs_timestamp");
  EXPECT_EQ(ct[2], hdf5::datatype::create<uint64_t>());

  EXPECT_EQ(ct.field_name(3), "channel");
  EXPECT_EQ(ct[3], hdf5::datatype::create<uint16_t>());

  EXPECT_EQ(ct.field_name(4), "bcid");
  EXPECT_EQ(ct[4], hdf5::datatype::create<uint16_t>());

  EXPECT_EQ(ct.field_name(5), "tdc");
  EXPECT_EQ(ct[5], hdf5::datatype::create<uint16_t>());

  EXPECT_EQ(ct.field_name(6), "adc");
  EXPECT_EQ(ct[6], hdf5::datatype::create<uint16_t>());

  EXPECT_EQ(ct.field_name(7), "over_threshold");
  EXPECT_EQ(ct[7], hdf5::datatype::create<bool>());

  EXPECT_EQ(ct.field_name(8), "chiptime");
  EXPECT_EQ(ct[8], hdf5::datatype::create<float>());
}

TEST_F(NMXReadoutTest, CreateFile) {
  ReadoutFile::create("readout_file_test");
  EXPECT_TRUE(hdf5::file::is_hdf5_file("readout_file_test_00000.h5"));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
