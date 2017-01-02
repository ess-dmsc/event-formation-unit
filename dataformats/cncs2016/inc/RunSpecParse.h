/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief parse json file to get run specifications
 */

#pragma once
#include <vector>
#include <RunSpec.h>

class RunSpecParse {
public:
  RunSpecParse(std::string jsonfile);

  std::vector<RunSpec*> & getruns(std::string runspec,
                 std::string basedir, std::string ofile, std::string cfile,
                 int start, int end);

private:
  std::string file;
  std::vector<RunSpec*> runs;
};
