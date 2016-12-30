/** Copyright (C) 2016 European Spallation Source ERIC */

#include <Analyse.h>
#include <Args.h>
#include <cassert>
#include <json.h>
#include <fstream>
#include <vector>

class Run {
public:
  Run(std::string dir, std::string prefix, std::string postfix,
      unsigned int start, unsigned int end, std::string ofile, std::string cfile)
        : dir_(dir)
        , prefix_(prefix)
        , postfix_(postfix)
        , start_(start)
        , end_(end)
        , ofile_(ofile)
        , cfile_(cfile) {};

  std::string dir_;
  std::string prefix_;
  std::string postfix_;
  unsigned int start_;
  unsigned int end_;
  std::string ofile_;
  std::string cfile_;
};


int main(int argc, char *argv[]) {
  Args opts(argc, argv);

  std::vector<Run *> Runs;

  if (!opts.runfile.empty()) {

    std::ifstream t(opts.runfile);
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());

    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(str, root, 0)) {
      printf("Cannot parse file %s as valid json\n", opts.runfile.c_str());
      exit(1);
    }

    const Json::Value plugins = root[opts.runspec];

    for (unsigned int index = 0; index < plugins.size(); index++) {
      auto runid = plugins[index]["id"].asInt();
      if ((runid >= opts.start && runid <= opts.end) || opts.start == 0) {
        auto dir = opts.basedir + plugins[index]["dir"].asString();
        auto pre = plugins[index]["prefix"].asString();
        auto pos = plugins[index]["postfix"].asString();
        auto start = plugins[index]["start"].asInt();
        auto end = plugins[index]["end"].asInt();
        Runs.push_back(new Run(dir, pre, pos, start, end,
                               opts.ofile + "/" + pre, opts.cfile + "/" + pre));
      }
    }
  } else {
    Runs.push_back(new Run(opts.basedir + opts.dir, opts.prefix, opts.postfix,
                   opts.start, opts.end, opts.ofile, opts.cfile));
  }

  // Start the analysis
  for (auto run : Runs) {
    opts.ofile = run->ofile_;
    opts.cfile = run->cfile_;
    Analyze analyze(opts);
    analyze.batchreader(run->dir_, run->prefix_, run->postfix_, run->start_, run->end_);
    analyze.makecal();
  }

}
