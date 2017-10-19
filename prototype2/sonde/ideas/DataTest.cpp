/** Copyright (C) 2017 European Spallation Source ERIC */

#include <algorithm>
#include <memory>
#include <sonde/Geometry.h>
#include <sonde/ideas/Data.h>
#include <sonde/ideas/TestData.h>
#include <test/TestBase.h>

using namespace std;

class IDEASDataTest : public TestBase {
protected:
  SoNDeGeometry geometry;
  IDEASData * readout;
  virtual void SetUp() {
     readout = new IDEASData(&geometry);
     memset(readout->data, 0, sizeof(readout->data));
  }
  virtual void TearDown() { }
};

/** Test cases below */
TEST_F(IDEASDataTest, ErrNoData) {
    int res = readout->parse_buffer(0, 20);
    ASSERT_EQ(res, -IDEASData::EBUFFER);
}

TEST_F(IDEASDataTest, ErrHeaderFields) {
  int i = 0;
  for (auto v : err_hdr) {
    MESSAGE() << "err_hdr[" << i++ << "]\n";
    int res = readout->parse_buffer((char *)&v[0], v.size());
    ASSERT_EQ(res, -IDEASData::EHEADER);
  }
}

TEST_F(IDEASDataTest, ErrShortHeader) {
    int size = err_short_header.size();
    int res = readout->parse_buffer((char *)&err_short_header[0], size);
    ASSERT_EQ(res, -IDEASData::EBADSIZE);
}

TEST_F(IDEASDataTest, OkHeaderOnly) {
    int size = ok_header_only.size();
    int res = readout->parse_buffer((char *)&ok_header_only[0], size);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(readout->events, 0);
}

TEST_F(IDEASDataTest, OkOneEvent) {
    int size = ok_events_1.size();
    int res = readout->parse_buffer((char *)&ok_events_1[0], size);
    ASSERT_EQ(res, 1);
    ASSERT_EQ(readout->events, 1);
}

TEST_F(IDEASDataTest, OkThreeEvents) {
    int size = ok_events_3.size();
    int res = readout->parse_buffer((char *)&ok_events_3[0], size);
    ASSERT_EQ(res, 3);
    ASSERT_EQ(readout->events, 3);

    for (int i = 0; i < res; i++) {
      ASSERT_NE(0, readout->data[i].time);
      ASSERT_NE(0, readout->data[i].pixel_id);
    }
}

TEST_F(IDEASDataTest, SEPHOkOneSample) {
    int size = type_0xd5_seph_ok_1.size();
    int res = readout->parse_buffer((char *)&type_0xd5_seph_ok_1[0], size);
    ASSERT_EQ(res, 1);
    ASSERT_EQ(readout->samples, 1);
}

TEST_F(IDEASDataTest, SEPHOkThreeSamples) {
    int size = type_0xd5_seph_ok_3.size();
    int res = readout->parse_buffer((char *)&type_0xd5_seph_ok_3[0], size);
    ASSERT_EQ(res, 1); // Should always return 0 events
    ASSERT_EQ(readout->samples, 3);
}

TEST_F(IDEASDataTest, MEPHOkOneSample) {
    int size = type_0xd4_meph_ok_1.size();
    int res = readout->parse_buffer((char *)&type_0xd4_meph_ok_1[0], size);
    ASSERT_EQ(res, 1); // Should always return 0 events
    ASSERT_EQ(readout->samples, 1);
}

TEST_F(IDEASDataTest, MEPHOkN3M1) {
    int size = type_0xd4_meph_ok_n3m1.size();
    int res = readout->parse_buffer((char *)&type_0xd4_meph_ok_n3m1[0], size);
    ASSERT_EQ(res, 3); // Should always return 0 events
    ASSERT_EQ(readout->samples, 3);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
