/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <RunSpecParse.h>
#include <fstream>
#include <string>

RunSpecParse::RunSpecParse(std::string jsonfile) : file(jsonfile) {}

  std::vector<RunSpec *> &RunSpecParse::getruns(std::string runspec,
                                              std::string basedir,
                                              std::string outputdir,
                                              int start,
                                              int end) {

  std::ifstream t(file);
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());

  if (basedir.back() != '/') {
    basedir += '/';
  }

  if (outputdir.back() != '/') {
    outputdir += '/';
  }

  if (!reader.parse(str, root, 0)) {
    printf("error: file \"%s\" is not valid json\n", file.c_str());
    exit(1);
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
      std::string outputfile = outputdir + runspec + "_" + std::to_string(runid);

      runs.push_back(new RunSpec(dir, pre, pos, start, end, outputfile, thresh));
    }
  }
  return runs;
}
