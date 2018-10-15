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
  }
  virtual void TearDown() {}
};

TEST_F(ReadoutFileTest, CreateFile) {
  ReadoutFile::create("readout_file_test");
  EXPECT_TRUE(hdf5::file::is_hdf5_file("readout_file_test.h5"));
}

// \todo test h5 compound type mapping

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
