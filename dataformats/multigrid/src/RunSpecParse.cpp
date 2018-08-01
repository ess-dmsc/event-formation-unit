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
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());

  if (basedir.back() != '/') {
    basedir += '/';
  }

  if (outputdir.back() != '/') {
    outputdir += '/';
  }

  if (!reader.parse(str, root, 0)) {
    printf("error: file \"%s\" is not valid json\n", jsonfile.c_str());
    throw std::runtime_error("Parsed file was not valid JSON.");
  }

  const Json::Value plugins = root[runspec];

  /** Run through all runs for the selected runspec group */
  for (unsigned int index = 0; index < plugins.size(); index++) {
    auto runid = plugins[index][ID].asInt();
    if ((runid >= start && runid <= end) || start == 0) {

      auto dir = basedir + plugins[index][DIR].asString();
      auto pre = plugins[index][PREFIX].asString();
      auto pos = plugins[index][POSTFIX].asString();
      auto start = plugins[index][START].asInt();
      auto end = plugins[index][END].asInt();

      /** If threshold is specified, use it. Else use default threshold alg. */
      int thresh = -1;
      auto object = plugins[index][THRESHOLD];
      if (object != Json::Value::null) {
        thresh = plugins[index][THRESHOLD].asInt();
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
