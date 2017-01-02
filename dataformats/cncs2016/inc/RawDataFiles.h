/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief list of files
 */

#pragma once

#include <string>
#include <vector>

class RunSpec {
public:
  int index;
  std::string sub_dir;
  std::string file_prefix;
  std::string file_postfix;
  int range_start;
  int range_end;
  int goodpeaks;
};

// clang-format off

std::vector<RunSpec> filelist {
    {1,  "07_11/beamOn_resetOn/",      "2016_07_11_beamOn1_",                    ".bin", 0,  1121, 0},
    {2,  "07_11/no_reset/",            "2016_07_11_background_",                 ".bin", 0,    59, 0},
    {3,  "07_12_background/",          "2016_07_12_beamOn2_",                    ".bin", 0,  1083, 0},
    {4,  "07_13_12A_Vanadium_powder/", "2016_07_14_12A_",                        ".bin", 0,    63, 0},
    {5,  "07_13_4p96A/",               "2016_07_13_beamOn_4p96A_",               ".bin", 0,   162, 1},
    {6,  "07_13_7p2A/",                "2016_07_13_beamOn_7p2A_",                ".bin", 0,   285, 0},
    {7,  "07_13_7p2A/1_t0_timing/",    "2016_07_13_beamOn_7p2A_",                ".bin", 0,    26, 0},
    {8,  "07_14/",                     "2016_07_14_2012_sample_",                ".bin", 0,    22, 0},
    {9,  "07_14/",                     "2016_07_14_2041_sample_",                ".bin", 0,    25, 1},
    {10, "07_14/",                     "2016_07_14_2133_sample_",                ".bin", 0,    78, 1},
    {11, "07_15/",                     "2016_07_14_1711_sample_",                ".bin", 0, 15153, 1},
    {12, "07_25/",                     "2016_07_25_1051_sample_",                ".bin", 0,  1365, 1},
    {13, "07_25/",                     "2016_07_26_1005_sample_",                ".bin", 0,  1442, 1},
    {14, "07_25/",                     "2016_07_27_1113_sample_",                ".bin", 0, 27901, 0},
    {15, "08_16/",                     "2016_08_16_0850_sample_",                ".bin", 0,     1, 0},
    {16, "08_16/",                     "2016_08_16_0907_sample_",                ".bin", 0,     1, 0},
    {17, "08_16/",                     "2016_08_16_0921_sample_",                ".bin", 0, 60477, 1},
    {18, "09_29/",                     "2016_09_29_0849_sample_",                ".bin", 0,  1250, 0},
    {19, "09_29/",                     "2016_09_30_0606_sample_",                ".bin", 0,  4195, 0},
    {20, "09_29/",                     "2016_10_03_0606_sample_",                ".bin", 0, 13785, 0},
    {21, "10_13/",                     "2016_10_13_0520_sample_",                ".bin", 0,  9933, 1},
    {22, "10_13/",                     "2016_10_20_0811_sample_",                ".bin", 0,  1145, 1},
    {23, "10_13/",                     "2016_10_21_0351_sample_",                ".bin", 0, 14597, 1},
    {24, "10_13/",                     "2016_10_31_1247_sample_",                ".bin", 0,  8190, 1},
    {25, "10_13/",                     "2016_11_07_0351_sample_",                ".bin", 0, 25476, 1},
    {26, "11_29/",                     "2016_11_30_0846_sample_",                ".bin", 0,  8285, 1},
    {27, "11_29/",                     "2016_12_06_0527_sample_",                ".bin", 0,    24, 1},
    {28, "11_29/",                     "2016_12_06_0555_sample_",                ".bin", 0,   183, 1},
    {29, "11_29/",                     "2016_12_06_1059_sample_",                ".bin", 0, 12615, 1}
};

std::vector<RunSpec> oldfilelist{
    {1, "/home/morten/cncsdata/vanadium_july_27/", "2016_07_25_1051_sample_", ".bin", 0,  1365, 1},
    {2, "/home/morten/cncsdata/vanadium_july_27/", "2016_07_26_1005_sample_", ".bin", 0,  1442, 1},
    {3, "/home/morten/cncsdata/vanadium_july_27/", "2016_07_27_1113_sample_", ".bin", 0, 27901, 0},
};
// clang-format on
