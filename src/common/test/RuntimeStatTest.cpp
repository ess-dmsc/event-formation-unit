/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/RuntimeStat.h>
#include <test/TestBase.h>

class RuntimeStatTest : public TestBase {
protected:
  uint32_t CtrA, CtrB, CtrC;
};

TEST_F(RuntimeStatTest, ConstructorBad) {
  EXPECT_ANY_THROW(RuntimeStat stats({}));
}

TEST_F(RuntimeStatTest, CounterMismatch) {
  RuntimeStat stats({CtrA, CtrB});
  uint32_t res = stats.getRuntimeStatus({});
  EXPECT_EQ(res, 0);
  res = stats.getRuntimeStatus({CtrA});
  EXPECT_EQ(res, 0);
  res = stats.getRuntimeStatus({CtrA, CtrB, CtrC});
  EXPECT_EQ(res, 0);
  res = stats.getRuntimeStatus({CtrA, CtrB, CtrC, CtrA});
  EXPECT_EQ(res, 0);
}

TEST_F(RuntimeStatTest, CounterGood) {
  RuntimeStat stats({CtrA, CtrB, CtrC});
  uint32_t res = stats.getRuntimeStatus({CtrA, CtrB, CtrC});
  EXPECT_EQ(res, 0);

  CtrA++;
  res = stats.getRuntimeStatus({CtrA, CtrB, CtrC});
  EXPECT_EQ(res, 1);

  CtrB++;
  res = stats.getRuntimeStatus({CtrA, CtrB, CtrC});
  EXPECT_EQ(res, 2);

  CtrC++;
  res = stats.getRuntimeStatus({CtrA, CtrB, CtrC});
  EXPECT_EQ(res, 4);

  CtrA++;
  CtrB++;
  CtrC++;
  res = stats.getRuntimeStatus({CtrA, CtrB, CtrC});
  EXPECT_EQ(res, 7);

  res = stats.getRuntimeStatus({CtrA, CtrB, CtrC});
  EXPECT_EQ(res, 0);
}



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
