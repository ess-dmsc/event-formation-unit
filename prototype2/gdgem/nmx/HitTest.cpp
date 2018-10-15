/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Hit.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class NMXHitTest : public TestBase {
protected:
  Hit hit;
  virtual void SetUp() {
    hdf5::error::Singleton::instance().auto_print(false);
    if (boost::filesystem::exists("hit_file_test.h5"))
    {
      boost::filesystem::remove("hit_file_test.h5");
    }
  }
  virtual void TearDown() { }
};

TEST_F(NMXHitTest, Debug) {
  ASSERT_FALSE(hit.debug().empty());
}

TEST_F(NMXHitTest, CreateFile) {
  HitFile::create("hit_file_test");
  EXPECT_TRUE(hdf5::file::is_hdf5_file("hit_file_test.h5"));
}

// \todo test h5 compound type mapping

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
