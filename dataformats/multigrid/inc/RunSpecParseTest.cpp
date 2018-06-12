/** Copyright (C) 2016 European Spallation Source ERIC */

#include <prototype2/common/DataSave.h>
#include <RunSpecParse.h>
#include <test/TestBase.h>
#include <cstring>


std::string filename{"deleteme.json"};

std::string runspecjson = " \
{ \n\
  \"test\" :\n\
    [ \
      {\"id\": 42,  \"dir\": \"mydir\",  \"prefix\": \"myprefix\",  \"postfix\": \"mypostfix\", \"start\": 1, \"end\":   999, \"thresh\": 124} \n\
    ] \n\
}";

class RunSpecParseTest : public TestBase {
protected:
};

/** Test cases below */

TEST_F(RunSpecParseTest, DefaultConstructor) {
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

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
