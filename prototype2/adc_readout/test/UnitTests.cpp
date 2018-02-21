/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Starts unit tests.
 */

#include <gtest/gtest.h>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
