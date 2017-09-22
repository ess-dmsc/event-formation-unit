/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Producer.h>
#include <cstring>
#include <gdgem/nmx/HistSerializer.h>
#include <test/TestBase.h>


class HistSerializerTest : public TestBase {
  virtual void SetUp() {
    for (int i = 0; i < NMX_STRIP_HIST_SIZE; i++) {
      hists.x_strips_hist[i] = i;
      hists.y_strips_hist[i] = NMX_STRIP_MAX_VAL - i;
    }
  }

  virtual void TearDown() {}

protected:
  NMXHists hists;
  char *buffer;
  char flatbuffer[1024 * 1024 * 5];
};

TEST_F(HistSerializerTest, Serialize) {
  HistSerializer histfb;
  auto len = histfb.serialize(hists, &buffer);
  ASSERT_TRUE(len > NMX_HIST_ELEM_SIZE * 2);
}

TEST_F(HistSerializerTest, DeSerialize) {
  HistSerializer histfb;

  auto length = histfb.serialize(hists, &buffer);

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
