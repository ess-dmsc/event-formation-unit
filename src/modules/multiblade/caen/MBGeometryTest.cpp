/** Copyright (C) 2018 European Spallation Source ERIC */

#include <multiblade/caen/MBGeometry.h>
#include <test/TestBase.h>

//using namespace Multiblade;

class MBGeometryTest : public TestBase {
protected:
  uint16_t ncass{6};
  uint16_t nw{32};
  uint16_t ns{32};
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(MBGeometryTest, Constructor) {
  MBGeometry geometry(ncass, nw, ns);
  ASSERT_TRUE(geometry.isFreia());
  ASSERT_FALSE(geometry.isEstia());
  ASSERT_TRUE(geometry.isDetectorMB18());
  ASSERT_FALSE(geometry.isDetectorMB16());
}

TEST_F(MBGeometryTest, DefaultGeometryBounds) {
  MBGeometry geometry(ncass, nw, ns);
  ASSERT_EQ(geometry.getPixel(32, 0), 0);
  ASSERT_NE(geometry.getPixel(31, 0), 0);
  ASSERT_EQ(geometry.getPixel( 0, 192), 0);
  ASSERT_NE(geometry.getPixel( 0, 191), 0);
}

TEST_F(MBGeometryTest, SetGeometryBounds) {
  MBGeometry geometry(ncass, nw, ns);
  geometry.setConfigurationEstia();
  ASSERT_EQ(geometry.getPixel(192, 0), 0);
  ASSERT_NE(geometry.getPixel(191, 0), 0);
  ASSERT_EQ(geometry.getPixel( 0, 32), 0);
  ASSERT_NE(geometry.getPixel( 0, 31), 0);

  geometry.setConfigurationFreia();
  ASSERT_EQ(geometry.getPixel(32, 0), 0);
  ASSERT_NE(geometry.getPixel(31, 0), 0);
  ASSERT_EQ(geometry.getPixel( 0, 192), 0);
  ASSERT_NE(geometry.getPixel( 0, 191), 0);
}

TEST_F(MBGeometryTest, EstiaPixelCorners) {
  MBGeometry geometry(ncass, nw, ns);
  geometry.setConfigurationEstia();
  /// cassette 0
  ASSERT_EQ(geometry.getPixel(0,  0), 1);
  ASSERT_EQ(geometry.getPixel(31, 0), 32);
  ASSERT_EQ(geometry.getPixel(0, 31), 5953);
  ASSERT_EQ(geometry.getPixel(31, 31),5984);
  /// cassette 5
  ASSERT_EQ(geometry.getPixel(160, 0), 161);
  ASSERT_EQ(geometry.getPixel(191, 0), 192);
  ASSERT_EQ(geometry.getPixel(160, 31), 6113);
  ASSERT_EQ(geometry.getPixel(191, 31), 6144);
}

TEST_F(MBGeometryTest, FreiaPixelCorners) {
  MBGeometry geometry(ncass, nw, ns);
  geometry.setConfigurationFreia();
  /// cassette 0
  ASSERT_EQ(geometry.getPixel(0,  0), 1);
  ASSERT_EQ(geometry.getPixel(31, 0), 32);
  ASSERT_EQ(geometry.getPixel(0, 31), 993);
  ASSERT_EQ(geometry.getPixel(31, 31),1024);
  /// cassette 5
  ASSERT_EQ(geometry.getPixel(0, 160), 5121);
  ASSERT_EQ(geometry.getPixel(31, 160), 5152);
  ASSERT_EQ(geometry.getPixel(0, 191), 6113);
  ASSERT_EQ(geometry.getPixel(31,191), 6144);
}

TEST_F(MBGeometryTest, isStripisWire) {
  MBGeometry geometry(ncass, nw, ns);
  for (unsigned int ch = 0; ch < 32; ch++) {
    ASSERT_TRUE(geometry.isWire(ch));
    ASSERT_FALSE(geometry.isStrip(ch));
  }
  for (unsigned int ch = 32; ch < 64; ch++) {
    ASSERT_FALSE(geometry.isWire(ch));
    ASSERT_TRUE(geometry.isStrip(ch));
  }
  for (unsigned int ch = 64; ch < 128; ch++) {
    ASSERT_FALSE(geometry.isWire(ch));
    ASSERT_FALSE(geometry.isStrip(ch));
  }
}


TEST_F(MBGeometryTest, XPositionsFreia) {
  MBGeometry geometry(ncass, nw, ns);
  geometry.setConfigurationFreia();
  MESSAGE() << "Testing getx() Freia MB16\n";
  geometry.setDetectorMB16();
  for (unsigned int cass = 0; cass < 9; cass++) {
    ASSERT_EQ(geometry.getx(cass, 32), 0); // left
    ASSERT_EQ(geometry.getx(cass, 33), 1);
    //        ...
    ASSERT_EQ(geometry.getx(cass, 62), 30);
    ASSERT_EQ(geometry.getx(cass, 63), 31); // right
  }

  MESSAGE() << "Testing getx() Freia MB18\n";
  geometry.setDetectorMB18();
  for (unsigned int cass = 0; cass < 9; cass++) {
    ASSERT_EQ(geometry.getx(cass, 33), 0); // left
    ASSERT_EQ(geometry.getx(cass, 32), 1); // left + one in x
    //        ...
    ASSERT_EQ(geometry.getx(cass, 63), 30); // right - one in x
    ASSERT_EQ(geometry.getx(cass, 62), 31); // right
  }
}

TEST_F(MBGeometryTest, YPositionsFreia) {
  MBGeometry geometry(ncass, nw, ns);
  geometry.setConfigurationFreia();
  MESSAGE() << "Testing gety() Freia MB16\n";
  geometry.setDetectorMB16();
  for (unsigned int cass = 0; cass < 9; cass++) {
    int yoff = cass * nw;
    ASSERT_EQ(geometry.gety(cass, 31), yoff + 0); // top
    ASSERT_EQ(geometry.gety(cass, 30), yoff + 1); // top + one in y
    //        ...
    ASSERT_EQ(geometry.gety(cass,  1), yoff + 30); // bottom - one in y
    ASSERT_EQ(geometry.gety(cass,  0), yoff + 31); // bottom
  }

  MESSAGE() << "Testing gety() Freia MB18\n";
  geometry.setDetectorMB18();
  for (unsigned int cass = 0; cass < 9; cass++) {
    int yoff = cass * nw;
    ASSERT_EQ(geometry.gety(cass,  1), yoff + 0); // top
    ASSERT_EQ(geometry.gety(cass,  0), yoff + 1); // top + one in y
    //        ...
    ASSERT_EQ(geometry.gety(cass, 31), yoff + 30); // bottom - one in y
    ASSERT_EQ(geometry.gety(cass, 30), yoff + 31); // bottom
  }
}

TEST_F(MBGeometryTest, XPositionsEstia) {
  MBGeometry geometry(ncass, nw, ns);
  geometry.setConfigurationEstia();
  MESSAGE() << "Testing getx() Estia MB16\n";
  geometry.setDetectorMB16();
  for (unsigned int cass = 0; cass < 6; cass++) {
    MESSAGE() << "cassette " << cass << "\n";
    auto xoff = cass * nw;
    ASSERT_EQ(geometry.getx(cass, 0), xoff + 0); // left
    ASSERT_EQ(geometry.getx(cass, 1), xoff + 1); // left + one in x
    //        ...
    ASSERT_EQ(geometry.getx(cass, 30), xoff + 30); // right - one in x
    ASSERT_EQ(geometry.getx(cass, 31), xoff + 31); // right
  }

  MESSAGE() << "Testing getx() Estia MB18\n";
  geometry.setDetectorMB18();
  for (unsigned int cass = 0; cass < 6; cass++) {
    MESSAGE() << "cassette " << cass << "\n";
    auto xoff = cass * nw;
    ASSERT_EQ(geometry.getx(cass, 30), xoff + 0); // left
    ASSERT_EQ(geometry.getx(cass, 31), xoff + 1); // left + one in x
    //        ...
    ASSERT_EQ(geometry.getx(cass, 0), xoff + 30); // right - one in x
    ASSERT_EQ(geometry.getx(cass, 1), xoff + 31); // right
  }
}

TEST_F(MBGeometryTest, YPositionsEstia) {
  MBGeometry geometry(ncass, nw, ns);
  geometry.setConfigurationEstia();
  MESSAGE() << "Testing gety() Estia MB16\n";
  geometry.setDetectorMB16();
  for (unsigned int cass = 0; cass < 9; cass++) {
    ASSERT_EQ(geometry.gety(cass, 32), 0); // top
    ASSERT_EQ(geometry.gety(cass, 33), 1); // top + one in y
    //        ...
    ASSERT_EQ(geometry.gety(cass, 62), 30); // bottom - one in y
    ASSERT_EQ(geometry.gety(cass, 63), 31); // bottom
  }

  MESSAGE() << "Testing gety() Estia MB18\n";
  geometry.setDetectorMB18();
  for (unsigned int cass = 0; cass < 9; cass++) {
    ASSERT_EQ(geometry.gety(cass, 33), 0); // top
    ASSERT_EQ(geometry.gety(cass, 32), 1); // top + one in y
    //        ...
    ASSERT_EQ(geometry.gety(cass, 63), 30); // bottom - one in y
    ASSERT_EQ(geometry.gety(cass, 62), 31); // bottom
  }
}

TEST_F(MBGeometryTest, GetPlane) {
  MBGeometry geometry(ncass, nw, ns);
  geometry.setConfigurationEstia();
  MESSAGE() << "For Estia wire channels are X plane\n";
  ASSERT_EQ(geometry.getPlane(0), 0);
  ASSERT_EQ(geometry.getPlane(nw - 1), 0);
  ASSERT_EQ(geometry.getPlane(nw), 1);
  ASSERT_EQ(geometry.getPlane(nw + ns - 1), 1);

  geometry.setConfigurationFreia();
  MESSAGE() << "For Estia wire channels are Y plane\n";
  ASSERT_EQ(geometry.getPlane(0), 1);
  ASSERT_EQ(geometry.getPlane(nw - 1), 1);
  ASSERT_EQ(geometry.getPlane(nw), 0);
  ASSERT_EQ(geometry.getPlane(nw + ns - 1), 0);
}

TEST_F(MBGeometryTest, GetGlobelChannel) {
  MBGeometry geometry(ncass, nw, ns);
  geometry.setConfigurationEstia();
  ASSERT_EQ(geometry.getGlobalChannel(0, 0), 0);
  ASSERT_EQ(geometry.getGlobalChannel(0, nw - 1), nw - 1);
  ASSERT_EQ(geometry.getGlobalChannel(0, 0), 0);
  ASSERT_EQ(geometry.getGlobalChannel(0, nw), 0);
  ASSERT_EQ(geometry.getGlobalChannel(0, nw + ns - 1), ns - 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
