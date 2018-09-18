/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief parse json file to get run specifications
///
//===----------------------------------------------------------------------===//

#pragma once
#include <dataformats/multigrid/inc/RunSpec.h>
#include <dataformats/multigrid/inc/json.h>
#include <vector>

class RunSpecParse {
public:
  /// \brief specify a json file containing run specifications (for Multigrid)
  RunSpecParse(std::string jsonfile);

  /// \brief create a list of runs from a given run specification
  std::vector<RunSpec *> &getruns(std::string runspec, std::string basedir,
                                  std::string outputdir, int start, int end);

private:
  Json::Value root{};    ///< for jsoncpp parser
  Json::Reader reader{}; ///< for jsoncpp parser
  std::string jsonfile{};
  std::vector<RunSpec *> runs{};

  const char *ID = "id";
  const char *DIR = "dir";
  const char *PREFIX = "prefix";
  const char *POSTFIX = "postfix";
  const char *START = "start";
  const char *END = "end";
  const char *THRESHOLD = "thresh";
};
