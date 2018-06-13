/** Copyright (C) 2016 European Spallation Source ERIC */

#include <prototype2/common/DataSave.h>
#include <RunSpecParse.h>
#include <test/TestBase.h>
#include <cstring>

std::string filename{"deleteme.json"};

std::string runspecjson = R"(
{
  "test" :
    [
      {"id": 42,  "dir": "mydir",  "prefix": "myprefix",  "postfix": "mypostfix", "start": 1, "end":   999, "thresh": 124}
    ]
})";

std::string invalidjson = R"({ "this is not valid json" })";

class RunSpecParseTest : public TestBase {
protected:
};

/** Test cases below */

TEST_F(RunSpecParseTest, ParseJson) {
  DataSave tempfile(filename, (void *)runspecjson.c_str(), runspecjson.size());
  RunSpecParse runspec(filename);

  auto runs = runspec.getruns("test", "basedir", "outputdir", 1, 200);

  ASSERT_EQ(1, runs.size());
  ASSERT_EQ(runs.at(0)->dir_, "basedir/mydir");
  ASSERT_EQ(runs.at(0)->ofile_, "outputdir/test_42");
  ASSERT_EQ(runs.at(0)->prefix_, "myprefix");
  ASSERT_EQ(runs.at(0)->postfix_, "mypostfix");
  ASSERT_EQ(runs.at(0)->start_, 1);
  ASSERT_EQ(runs.at(0)->end_, 999);
}

TEST_F(RunSpecParseTest, InvalidJson) {
  DataSave tempfile(filename, (void *)invalidjson.c_str(), invalidjson.size());
  RunSpecParse runspec(filename);

  // DeathTest
  ASSERT_EXIT(runspec.getruns("test", "basedir", "outputdir", 1, 200), ::testing::ExitedWithCode(1), "");
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
