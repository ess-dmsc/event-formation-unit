/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/mesytec/Readout.h>
#include <common/testutils/TestBase.h>

using namespace Multigrid;

class MGHitTest : public TestBase {
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

TEST_F(MGHitTest, PrintsSelf) {
  Readout h;
  EXPECT_FALSE(h.debug().empty());
  // Don't really care about particular contents here
}

TEST_F(MGHitTest, CompoundMapping) {
  // If you are forced to change anything here,
  // you have broken dumpfile compatibility, and you should
  // bump FormatVersion for the struct

  auto t = hdf5::datatype::create<Readout>();

  EXPECT_EQ(t.number_of_fields(), 10ul);
  EXPECT_EQ(t.get_class(), hdf5::datatype::Class::COMPOUND);

  auto ct = hdf5::datatype::Compound(t);

  EXPECT_EQ(ct.field_name(0), "trigger_count");
  EXPECT_EQ(ct[0], hdf5::datatype::create<size_t>());

  EXPECT_EQ(ct.field_name(1), "external_trigger");
  EXPECT_EQ(ct[1], hdf5::datatype::create<int8_t>());

  EXPECT_EQ(ct.field_name(2), "module");
  EXPECT_EQ(ct[2], hdf5::datatype::create<uint8_t>());

  EXPECT_EQ(ct.field_name(3), "high_time");
  EXPECT_EQ(ct[3], hdf5::datatype::create<uint32_t>());

  EXPECT_EQ(ct.field_name(4), "low_time");
  EXPECT_EQ(ct[4], hdf5::datatype::create<uint32_t>());

  EXPECT_EQ(ct.field_name(5), "total_time");
  EXPECT_EQ(ct[5], hdf5::datatype::create<uint64_t>());

  EXPECT_EQ(ct.field_name(6), "bus");
  EXPECT_EQ(ct[6], hdf5::datatype::create<uint8_t>());

  EXPECT_EQ(ct.field_name(7), "channel");
  EXPECT_EQ(ct[7], hdf5::datatype::create<uint16_t>());

  EXPECT_EQ(ct.field_name(8), "adc");
  EXPECT_EQ(ct[8], hdf5::datatype::create<uint16_t>());

  EXPECT_EQ(ct.field_name(9), "time_diff");
  EXPECT_EQ(ct[9], hdf5::datatype::create<uint16_t>());
}

TEST_F(MGHitTest, CreateFile) {
  ReadoutFile::create("hit_file_test");
  EXPECT_TRUE(hdf5::file::is_hdf5_file("hit_file_test_00000.h5"));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
