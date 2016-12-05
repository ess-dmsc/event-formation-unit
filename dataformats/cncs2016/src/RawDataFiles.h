/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief list of files
 */

 #pragma once

 #include <string>
 #include <vector>

 class Filelist {
 public:
   std::string base_dir;
   std::string file_prefix;
   std::string file_postfix;
   int range_start;
   int range_end;
 };

std::vector<Filelist> filelist {
  {"/home/morten/cncsdata/vanadium_july_27/", "2016_07_25_1051_sample_", ".bin", 0, 1365},
  {"/home/morten/cncsdata/vanadium_july_27/", "2016_07_26_1005_sample_", ".bin", 0, 1442},
  {"/home/morten/cncsdata/vanadium_july_27/", "2016_07_27_1113_sample_", ".bin", 0, 27901},
};
