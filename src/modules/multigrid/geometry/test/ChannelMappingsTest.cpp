/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/geometry/ChannelMappings.h>
#include <common/testutils/TestBase.h>

using namespace Multigrid;

class MockMappings : public ChannelMappings {
public:
  // Implementation

  uint16_t max_channel() const override {
    return 0;
  }

  uint16_t max_wire() const override {
    return 0;
  }

  uint16_t max_grid() const override {
    return 0;
  }

  bool isWire(uint16_t channel) const override {
    (void) channel;
    return true;
  }

  bool isGrid(uint16_t channel) const override {
    (void) channel;
    return true;
  }

  uint16_t wire(uint16_t channel) const override {
    (void) channel;
    return 0;
  }

  uint16_t grid(uint16_t channel) const override {
    (void) channel;
    return 0;
  }
};

class MGMappingsTest : public TestBase {
protected:
  MockMappings geo;
  Filter f;
  void SetUp() override {
    f.minimum = 3;
    f.maximum = 7;
    f.rescale_factor = 0.5;
  }
  void TearDown()  override {
  }
};

// \todo actual tests

TEST_F(MGMappingsTest, PrintsSelf) {
  geo.wire_filters.set_filters(1, f);
  geo.grid_filters.set_filters(1, f);
  EXPECT_FALSE(geo.debug({}).empty());
}

TEST_F(MGMappingsTest, FromJson) {
  nlohmann::json j;

  nlohmann::json b;
  b["min"] = 3;
  b["max"] = 7;
  b["rescale"] = 0.5;
  b["count"] = 10;
  j["wire_filters"]["blanket"] = b;

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
