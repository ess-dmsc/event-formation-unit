/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <dataformats/multigrid/inc/RunSpecParse.h>
#include <fstream>
#include <iomanip>
#include <string>
#include <stdexcept>

RunSpecParse::RunSpecParse(std::string filename) : jsonfile(filename) {}

std::vector<RunSpec *> &RunSpecParse::getruns(std::string runspec,
                                              std::string basedir,
                                              std::string outputdir, int start,
                                              int end) {

  std::ifstream t(jsonfile);
  std::string jsonstring((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());

  if (basedir.back() != '/') {
    basedir += '/';
  }

  if (outputdir.back() != '/') {
    outputdir += '/';
  }

  nlohmann::json root;
  try {
    root = nlohmann::json::parse(jsonstring);
  }
  catch (...) {
    printf("Invalid Json file: %s\n", jsonstring.c_str());
    throw std::runtime_error("Parsed file was not valid JSON.");
  }

  auto specs = root[runspec];

  /** Run through all runs for the selected runspec group */
  for (unsigned int i = 0; i < specs.size(); i++) {
    auto runid = specs[i][ID].get<int>();
    if ((runid >= start && runid <= end) || start == 0) {

      auto dir = basedir + specs[i][DIR].get<std::string>();
      auto pre = specs[i][PREFIX].get<std::string>();
      auto pos = specs[i][POSTFIX].get<std::string>();
      auto start = specs[i][START].get<int>();
      auto end = specs[i][END].get<int>();

      /** If threshold is specified, use it. Else use default threshold alg. */
      int thresh = -1;
      auto object = specs[i][THRESHOLD];
      if (!object.is_null()) {
        thresh = specs[i][THRESHOLD].get<int>();
      }

      // Make unique filename from runspec and runid
      std::ostringstream seqno;
      seqno.width(2);
      seqno << std::setfill('0') << runid;
      std::string outputfile = outputdir + runspec + "_" + seqno.str();

      runs.push_back(
          new RunSpec(dir, pre, pos, start, end, outputfile, thresh));
    }
  }
  return runs;
}
