/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Producer.h>
#include <cstring>
#include <gdgem/nmx/HistSerializer.h>
#include <test/TestBase.h>


class HistSerializerTest : public TestBase {
  virtual void SetUp() {
    for (int i = 0; i < NMX_STRIP_HIST_SIZE; i++) {
      xarr[i] = i;
      yarr[i] = NMX_STRIP_MAX_VAL - i;
    }
  }

  virtual void TearDown() {}

protected:
  NMX_HIST_TYPE xarr[NMX_STRIP_HIST_SIZE];
  NMX_HIST_TYPE yarr[NMX_STRIP_HIST_SIZE];
  char *buffer;
  char flatbuffer[1024 * 1024];
};

TEST_F(HistSerializerTest, Serialize) {
  for (int i = 0; i < NMX_STRIP_HIST_SIZE; i++) {
    HistSerializer histfb(i);
    auto len = histfb.serialize(xarr, yarr, i, &buffer);
    ASSERT_TRUE(len > i * NMX_HIST_ELEM_SIZE * 2);

    len = histfb.serialize(&xarr[0], &yarr[0], i + 1, &buffer);
    ASSERT_EQ(len, 0);
    ASSERT_EQ(buffer, nullptr);
  }
}

TEST_F(HistSerializerTest, DeSerialize) {
  HistSerializer histfb(NMX_STRIP_HIST_SIZE);

  auto length = histfb.serialize(&xarr[0], &yarr[0], NMX_STRIP_HIST_SIZE, &buffer);

  memcpy(flatbuffer, buffer, length);
  auto monitor = GetMonitorMessage(flatbuffer);
  auto dtype = monitor->data_type();
  ASSERT_EQ(dtype, DataField::GEMHist);

  auto hist = static_cast<const GEMHist *>(monitor->data());
  auto xdat = hist->xstrips();
  auto ydat = hist->ystrips();
  ASSERT_EQ(xdat->size(), NMX_STRIP_HIST_SIZE);
  ASSERT_EQ(ydat->size(), NMX_STRIP_HIST_SIZE);

  for (int i = 0; i < NMX_STRIP_HIST_SIZE; i++) {
    ASSERT_EQ((*xdat)[i], i);
    EXPECT_EQ((*ydat)[i], NMX_STRIP_MAX_VAL - i);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
