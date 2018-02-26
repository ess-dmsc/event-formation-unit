/** Copyright (C) 2016 European Spallation Source ERIC */

#include <common/DataSave.h>
#include <test/TestBase.h>
#include <sys/stat.h>

class DataSaveTest : public TestBase {
protected:
};

int getfilesize(std::string filename) {
  struct stat st;
  int ret = stat(filename.c_str(), &st);
  if (ret < 0) {
    printf("stat() returns %d\n", ret);
    return ret;
  }
  int size = st.st_size;
  return size;
}

/** Test cases below */

// Test that buffered write is working
TEST_F(DataSaveTest, BufferedWrite) {
  DataSave data("BufferedWrite", 50000000);

  char buffer[2000];

  for (int i = 0; i < 10; i++) {
    memset(buffer, 0x41 + i, sizeof(buffer));
    buffer[1999] = '\0';
    int ret = data.tofile("%s", buffer);
    ASSERT_EQ(ret, 0);
  }
  int ret = data.tofile("%s", buffer);
  ASSERT_EQ(ret, 11 * 1999);
}

// Test that buffer overrun cannot happen
TEST_F(DataSaveTest, BufferedSaveOverrun) {
  DataSave data("BufferedSaveOverrun", 50000000);

  char buffer[2000];
  char buffer2[4000];

  memset(buffer2, 0x41, sizeof(buffer2));
  buffer2[3999] = '\0';

  for (int i = 0; i < 10; i++) {
    memset(buffer, 0x41 + i, sizeof(buffer));
    buffer[1999] = '\0';
    int ret = data.tofile("%s", buffer);
    ASSERT_EQ(ret, 0);
  }
  int ret = data.tofile("%s", buffer2);
  ASSERT_EQ(ret, 22000); // private: BUFFERSIZE + MARGIN
}

// Test that unwritten buffer is written on exit
// Checking file sizes
TEST_F(DataSaveTest, FlushBuffer) {
  DataSave *data = new DataSave("FlushBuffer", 50000000);

  std::string name = data->getfilename();
  MESSAGE() << "Filename: " << name << std::endl;

  char buffer[2000];

  for (int i = 0; i < 10; i++) {
    memset(buffer, 0x41 + i, sizeof(buffer));
    buffer[1999] = '\0';
    int ret = data->tofile("%s", buffer);
    ASSERT_EQ(ret, 0);
  }

  MESSAGE() << "Checking filesized before and after destructor" << std::endl;
  ASSERT_EQ(0, getfilesize(name));
  delete (data);
  ASSERT_EQ(10 * 1999, getfilesize(name));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
