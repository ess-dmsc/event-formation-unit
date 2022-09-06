/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/testutils/TestBase.h>
#include <multigrid/geometry/MG24Mappings.h>

class MG24MappingsTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

/** Test cases below */

TEST_F(MG24MappingsTest, WireOrGridVariantA) {
  Multigrid::MG24MappingsA mgdet;
  mgdet.max_channel(128);

  for (int i = 0; i <= 79; i++) {
    EXPECT_TRUE(mgdet.isWire(i));
    EXPECT_FALSE(mgdet.isGrid(i));
  }

  for (int i = 80; i <= 127; i++) {
    EXPECT_FALSE(mgdet.isWire(i)) << " bad wire eval at " << i;
    EXPECT_TRUE(mgdet.isGrid(i)) << " bad wire eval at " << i;
  }

  EXPECT_FALSE(mgdet.isWire(128));
  EXPECT_FALSE(mgdet.isGrid(128));
}

// \todo these tests are possibly more confusing than the implementation being
// tested

TEST_F(MG24MappingsTest, WiresVariantA) {
  Multigrid::MG24MappingsA mgdet;
  mgdet.max_channel(128);

  for (uint16_t offset = 0; offset < 4; offset++) {
    MESSAGE() << "Lower wires: " << offset * 16 << " to " << (offset * 16 + 15)
              << "\n";
    for (uint16_t chan = 0; chan < 16; chan++) {
      uint16_t channel = offset * 16 + chan;
      EXPECT_EQ(channel + 4 * offset, mgdet.wire(channel))
          << " for offset=" << offset << " chan=" << chan << "\n";
    }
  }

  for (uint16_t offset = 0; offset < 4; offset++) {
    MESSAGE() << "Upper wires: " << 64 + offset * 4 << " to "
              << (64 + offset * 4 + 3) << "\n";
    for (uint16_t chan = 0; chan < 4; chan++) {
      uint16_t channel = 64 + offset * 4 + chan;
      uint16_t wire = channel - (3 - offset) * 16;
      EXPECT_EQ(wire, mgdet.wire(channel))
          << " for offset=" << offset << " chan=" << chan << "\n";
    }
  }
}

TEST_F(MG24MappingsTest, GridsVariantA) {
  Multigrid::MG24MappingsA mgdet;
  mgdet.max_channel(127);

  for (uint16_t channel = 80; channel < 127; channel++) {
    EXPECT_EQ(channel - 80, mgdet.grid(channel));
  }
}

// \todo tests for VariantB

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
