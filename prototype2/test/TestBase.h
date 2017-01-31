/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Base class for all unit tests - provides colored print using MESSAGE
 */

#include <gtest/gtest.h>

namespace testing {
namespace internal {
enum GTestColor { COLOR_DEFAULT, COLOR_RED, COLOR_GREEN, COLOR_YELLOW };

extern void ColoredPrintf(GTestColor color, const char *fmt, ...);
}
}

class TestBase : public ::testing::Test {
protected:
  class Message : public std::stringstream {
  public:
    ~Message() { //{cout << "[          ] " << str() << endl;}
      testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN,
                                       "[          ] ");
      testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, "%s",
                                       str().c_str());
    }
  };
#define MESSAGE Message
};
