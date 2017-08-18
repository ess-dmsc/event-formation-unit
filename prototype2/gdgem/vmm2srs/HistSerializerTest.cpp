/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Producer.h>
#include <cstring>
#include <gdgem/vmm2srs/HistSerializer.h>
#include <test/TestBase.h>

#define HISTSIZE 1500
#define ELEMSIZE 4

class HistSerializerTest : public TestBase {
  virtual void SetUp() {
    for (int i = 0; i < HISTSIZE; i++) {
      xarr[i] = i;
      yarr[i] = 200000 - i;
    }
  }

  virtual void TearDown() {}

protected:
  uint32_t xarr[HISTSIZE];
  uint32_t yarr[HISTSIZE];
  char *buffer;
  char flatbuffer[1024 * 1024];
};

TEST_F(HistSerializerTest, Serialize) {
  for (int i = 0; i <= HISTSIZE; i++) {
    HistSerializer histfb(i);
    auto len = histfb.serialize(xarr, yarr, i, &buffer);
    ASSERT_TRUE(len > i * ELEMSIZE * 2);

    len = histfb.serialize(xarr, yarr, i + 1, &buffer);
    ASSERT_EQ(len, 0);
    ASSERT_EQ(buffer, nullptr);
  }
}

TEST_F(HistSerializerTest, DeSerialize) {
  HistSerializer histfb(HISTSIZE);

  auto length = histfb.serialize(xarr, yarr, HISTSIZE, &buffer);

  memcpy(flatbuffer, buffer, length);
  auto monitor = GetMonitorMessage(flatbuffer);
  auto dtype = monitor->data_type();
  ASSERT_EQ(dtype, DataField_GEMHist);

  auto hist = static_cast<const GEMHist *>(monitor->data());
  auto xdat = hist->xhist();
  auto ydat = hist->yhist();
  ASSERT_EQ(xdat->size(), HISTSIZE);
  ASSERT_EQ(ydat->size(), HISTSIZE);

  for (int i = 0; i < HISTSIZE; i++) {
    ASSERT_EQ((*xdat)[i], i);
    ASSERT_EQ((*ydat)[i], 200000 - i);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
