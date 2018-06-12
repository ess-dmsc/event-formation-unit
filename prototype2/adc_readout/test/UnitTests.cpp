/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Starts unit tests.
 */

#include <gtest/gtest.h>
#include <trompeloeil.hpp>

namespace trompeloeil {
template <>
void reporter<specialized>::send(severity s, char const *file,
                                 unsigned long line, const char *msg) {
  if (s == severity::fatal) {
    std::ostringstream os;
    if (line != 0U) {
      os << file << ':' << line << '\n';
    }
    throw expectation_violation(os.str() + msg);
  }
  ADD_FAILURE_AT(file, line) << msg;
}
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
