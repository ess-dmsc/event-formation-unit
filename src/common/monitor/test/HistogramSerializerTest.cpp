/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/monitor/HistogramSerializer.h>

#include <common/testutils/TestBase.h>

#define MAX_STRIP_VAL_TEST 5000

class HistogramSerializerTest : public TestBase {
  void SetUp() override {
    for (size_t i = 0; i < hists.x_strips_hist.size(); i++) {
      hists.x_strips_hist[i] = i;
      hists.y_strips_hist[i] = MAX_STRIP_VAL_TEST - i;
    }
  }

  void TearDown() override {}

protected:
  Hists hists{MAX_STRIP_VAL_TEST, MAX_STRIP_VAL_TEST};
  char flatbuffer[1024 * 1024 * 5];

public:
  void copy_buffer(nonstd::span<const uint8_t> b) {
    memcpy(flatbuffer, b.data(), b.size_bytes());
  }
};

TEST_F(HistogramSerializerTest, Serialize) {
  HistogramSerializer histfb(hists.needed_buffer_size(), "some_source");
  auto len = histfb.produce(hists);
  EXPECT_GE(len, hists.needed_buffer_size());
}

TEST_F(HistogramSerializerTest, DeSerialize) {
  HistogramSerializer histfb(hists.needed_buffer_size(), "some_source");

  auto Produce = [this](auto DataBuffer, auto) {
    this->copy_buffer(DataBuffer);
  };

  histfb.set_callback(Produce);

  histfb.produce(hists);
  EXPECT_EQ(std::string(&flatbuffer[4], 4), "mo01");

  auto monitor = GetMonitorMessage(flatbuffer);
  EXPECT_EQ(monitor->source_name()->str(), "some_source");
  EXPECT_EQ(monitor->data_type(), DataField::GEMHist);

  auto hist = static_cast<const GEMHist *>(monitor->data());
  auto xdat = hist->xstrips();
  auto ydat = hist->ystrips();
  EXPECT_EQ(xdat->size(), hists.x_strips_hist.size());
  EXPECT_EQ(ydat->size(), hists.y_strips_hist.size());

  for (size_t i = 0; i < hists.x_strips_hist.size(); i++) {
    EXPECT_EQ((*xdat)[i], i);
    EXPECT_EQ((*ydat)[i], MAX_STRIP_VAL_TEST - i);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
