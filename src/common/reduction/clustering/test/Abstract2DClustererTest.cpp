// Copyright (C) 2017 European Spallation Source ERIC

#include <common/reduction/clustering/Abstract2DClusterer.h>

#include <common/testutils/TestBase.h>

class Abstract2DClustererTest : public TestBase {
protected:

};


TEST_F(Abstract2DClustererTest, Constructor) {
  Cluster2DContainer clusters;
  std::string DebugStr = to_string(clusters, "", true);
  size_t OldLen = DebugStr.size();
  DebugStr = to_string(clusters, "prepend_", true);
  ASSERT_TRUE(DebugStr.size() >= OldLen);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
