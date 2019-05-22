/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/geometry/ModuleGeometry.h>
#include <test/TestBase.h>

using namespace Multigrid;

class MockGeometry : public ModuleGeometry {
public:
  // Implementation

  uint16_t max_channel() const override {
    return 0;
  }

  uint16_t max_wire() const override {
    return 100;
  }

  uint16_t max_grid() const override {
    return 100;
  }

  uint32_t max_x() const override {
    return 0;
  }

  uint32_t max_y() const override {
    return 0;
  }

  uint16_t max_z() const override {
    return 0;
  }

  bool isWire(uint8_t VMM, uint16_t channel) const override {
    (void) VMM;
    (void) channel;
    return true;
  }

  bool isGrid(uint8_t VMM, uint16_t channel) const override {
    (void) VMM;
    (void) channel;
    return true;
  }

  uint16_t wire(uint8_t VMM, uint16_t channel) const override {
    (void) VMM;
    (void) channel;
    return 0;
  }

  uint16_t grid(uint8_t VMM, uint16_t channel) const override {
    (void) VMM;
    (void) channel;
    return 0;
  }

  uint32_t x_from_wire(uint16_t w) const override {
    (void) w;
    return 0;
  }

  uint32_t x(uint8_t VMM, uint16_t channel) const override {
    (void) VMM;
    (void) channel;
    return 0;
  }

  uint32_t y_from_grid(uint16_t g) const override {
    (void) g;
    return 0;
  }

  uint32_t y(uint8_t VMM, uint16_t channel) const override {
    (void) VMM;
    (void) channel;
    return 0;
  }

  uint32_t z_from_wire(uint16_t w) const override {
    (void) w;
    return 0;
  }

  uint32_t z(uint8_t VMM, uint16_t channel) const override {
    (void) VMM;
    (void) channel;
    return 0;
  }
};

class ModuleGeometryTest : public TestBase {
protected:
  MockGeometry geo;
  Filter f;
  virtual void SetUp() {
    f.minimum = 3;
    f.maximum = 7;
    f.rescale_factor = 0.5;
  }
  virtual void TearDown() {
  }
};

TEST_F(ModuleGeometryTest, OneWireFilter) {
  geo.override_wire_filter(5, f);

  EXPECT_EQ(geo.rescale_wire(4, 2), 2);
  EXPECT_TRUE(geo.valid_wire(4, 10));

  EXPECT_EQ(geo.rescale_wire(5, 2), 1);
  EXPECT_FALSE(geo.valid_wire(5, 10));

  EXPECT_EQ(geo.rescale_wire(6, 2), 2);
  EXPECT_TRUE(geo.valid_wire(6, 10));
}

TEST_F(ModuleGeometryTest, BlanketWireFilter) {
  geo.set_wire_filters(f);

  EXPECT_EQ(geo.rescale_wire(1, 2), 1);
  EXPECT_FALSE(geo.valid_wire(1, 10));

  EXPECT_EQ(geo.rescale_wire(5, 2), 1);
  EXPECT_FALSE(geo.valid_wire(5, 10));

  EXPECT_EQ(geo.rescale_wire(70, 2), 1);
  EXPECT_FALSE(geo.valid_wire(70, 10));
}

TEST_F(ModuleGeometryTest, OneGridFilter) {
  geo.override_grid_filter(5, f);

  EXPECT_EQ(geo.rescale_grid(4, 2), 2);
  EXPECT_TRUE(geo.valid_grid(4, 10));

  EXPECT_EQ(geo.rescale_grid(5, 2), 1);
  EXPECT_FALSE(geo.valid_grid(5, 10));

  EXPECT_EQ(geo.rescale_grid(6, 2), 2);
  EXPECT_TRUE(geo.valid_grid(6, 10));
}

TEST_F(ModuleGeometryTest, BlanketGridFilter) {
  geo.set_grid_filters(f);

  EXPECT_EQ(geo.rescale_grid(1, 2), 1);
  EXPECT_FALSE(geo.valid_grid(1, 10));

  EXPECT_EQ(geo.rescale_grid(5, 2), 1);
  EXPECT_FALSE(geo.valid_grid(5, 10));

  EXPECT_EQ(geo.rescale_grid(30, 2), 1);
  EXPECT_FALSE(geo.valid_grid(30, 10));
}

TEST_F(ModuleGeometryTest, PrintsSelf) {
  geo.set_wire_filters(f);
  geo.set_grid_filters(f);
  EXPECT_FALSE(geo.debug({}).empty());
}

TEST_F(ModuleGeometryTest, FromJson) {
  nlohmann::json j;

  auto j1 = j["wire_filters"]["blanket"];
  j1["min"] = 3;
  j1["max"] = 7;
  j1["rescale"] = 0.5;

  nlohmann::json j2;
  j2["idx"] = 5;
  j2["min"] = 3;
  j2["max"] = 7;
  j2["rescale"] = 0.5;
  j["wire_filters"]["exceptions"].push_back(j2);

  geo = j;

  // \todo test correct parsing
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
