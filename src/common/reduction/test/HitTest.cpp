// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file

#include <common/reduction/Hit.h>
#include <common/testutils/TestBase.h>

class HitTest : public TestBase {
protected:
  Hit hit;
  void SetUp() override {
    }
  
  void TearDown() override {}
};

TEST_F(HitTest, Debug) { EXPECT_FALSE(hit.to_string().empty()); }

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
