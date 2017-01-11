/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief parse json file to get run specifications
 */

#pragma once
#include <RunSpec.h>
#include <json.h>
#include <vector>

class RunSpecParse {
public:
  /** @todo document */
  RunSpecParse(std::string jsonfile);

  /** @todo document */
  std::vector<RunSpec *> &getruns(std::string runspec, std::string basedir,
                                  std::string outputdir, int start, int end);

private:
  Json::Value root{};    /**< for jsoncpp parser */
  Json::Reader reader{}; /**< for jsoncpp parser */
  std::string jsonfile{};
  std::vector<RunSpec *> runs{};

  const char * ID = "id";
  const char * DIR = "dir";
  const char * PREFIX = "prefix";
  const char * POSTFIX = "postfix";
  const char * START = "start";
  const char * END = "end";
  const char * THRESHOLD = "thresh";
};
