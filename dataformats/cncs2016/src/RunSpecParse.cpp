/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <RunSpecParse.h>
#include <fstream>
#include <json.h>
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

  Json::Value root;
  Json::Reader reader;

  if (!reader.parse(str, root, 0)) {
    printf("Cannot parse file %s as valid json\n", file.c_str());
    exit(1);
  }

  const Json::Value plugins = root[runspec];

  for (unsigned int index = 0; index < plugins.size(); index++) {
    auto runid = plugins[index]["id"].asInt();
    if ((runid >= start && runid <= end) || start == 0) {
      auto dir = basedir + plugins[index]["dir"].asString();
      auto pre = plugins[index]["prefix"].asString();
      auto pos = plugins[index]["postfix"].asString();
      auto start = plugins[index]["start"].asInt();
      auto end = plugins[index]["end"].asInt();

      int thresh = -1;
      auto object = plugins[index]["thresh"];
      if (object == Json::Value::null) {
        printf("No value for thresh use default\n");
      } else {
        thresh = plugins[index]["thresh"].asInt();
      }

      // Make unique filename from runspec and runid
      std::string outputfile = outputdir + "/" + runspec + "_" + std::to_string(runid);
      std::string outputcalib = outputfile;

      runs.push_back(new RunSpec(dir, pre, pos, start, end, outputfile, thresh));
    }
  }
  return runs;
}
