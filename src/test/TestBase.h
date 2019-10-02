/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Base class for all unit tests - provides colored print using MESSAGE
///
//===----------------------------------------------------------------------===//

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wsign-compare"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

namespace testing {
namespace internal {
enum GTestColor { COLOR_DEFAULT, COLOR_RED, COLOR_GREEN, COLOR_YELLOW };

extern void ColoredPrintf(GTestColor color, const char *fmt, ...);
} // namespace internal
} // namespace testing

class TestBase : public ::testing::Test {
protected:
  class Message : public std::stringstream {
  public:

  };
#define MESSAGE Message
};

// //Minimal Code for adding a test
// #include <test/TestBase.h>
//
// class MyNewTest : public TestBase {};
//
// TEST_F(MyNewTest, FirstTest) {
// }
//
// int main(int argc, char **argv) {
//   testing::InitGoogleTest(&argc, argv);
//   return RUN_ALL_TESTS();
// }
