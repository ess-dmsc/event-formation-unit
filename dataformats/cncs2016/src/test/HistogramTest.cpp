/** Copyright (C) 2016 European Spallation Source ERIC */

#include <Histogram.h>
#include <TestBase.h>

class HistogramTest : public TestBase {
protected:
};

/** Test cases below */

TEST_F(HistogramTest, Constructor) {
   Histogram hist;
   for (unsigned int i = 0; i < Histogram::histsize; i++) {
     ASSERT_EQ(0, hist.hist[i]);
   }
}

TEST_F(HistogramTest, SetClear) {
   Histogram hist;
   for (unsigned int i = 0; i < Histogram::histsize; i++) {
     hist.hist[i] = i;
   }
   for (unsigned int i = 0; i < Histogram::histsize; i++) {
     ASSERT_EQ(i, hist.hist[i]);
   }
   hist.clear();
   for (unsigned int i = 0; i < Histogram::histsize; i++) {
     ASSERT_EQ(0, hist.hist[i]);
   }
}

TEST_F(HistogramTest, Add) {
   Histogram hist;
   for (unsigned int i = 0; i < Histogram::histsize; i++) {
     hist.add(42);
   }
   auto hsize = Histogram::histsize;
   ASSERT_EQ(hsize, hist.hist[42]);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
