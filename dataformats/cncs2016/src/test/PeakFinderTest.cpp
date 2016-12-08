/** Copyright (C) 2016 European Spallation Source ERIC */

#include <Histogram.h>
#include <TestBase.h>
#include <PeakFinder.h>
#include "PeakFinderTestData.h"

class PeakFinderTest : public TestBase {
protected:
};

/** Test cases below */

TEST_F(PeakFinderTest, Constructor) {
   PeakFinder f(1, 2);
   ASSERT_EQ(1, f.getminwidth());
   ASSERT_EQ(2, f.getthresh());
   ASSERT_EQ(0, f.getcapped());
}

TEST_F(PeakFinderTest, Capped) {
   MESSAGE() << "Expecting no data to be capped\n";
   PeakFinder f(0, -1);
   auto peaks = f.findpeaks(testdata);
   ASSERT_EQ(0, f.getcapped());

   MESSAGE() << "Expecting all data to be capped\n";
   PeakFinder g(0, 20000);
   peaks = g.findpeaks(testdata);
   ASSERT_EQ(testdata.size(), g.getcapped());
}

TEST_F(PeakFinderTest, ValidateFromTestData) {
   PeakFinder f(1, 1);
   auto peaks = f.findpeaks(testdata);
   MESSAGE() << "Expecting 128 peaks in test dataset\n";
   ASSERT_EQ(128, peaks.size());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
