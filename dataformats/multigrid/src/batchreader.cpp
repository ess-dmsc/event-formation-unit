/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <Analyse.h>
#include <Args.h>
#include <RunSpecParse.h>

int main(int argc, char *argv[]) {
  Args opts(argc, argv);

  std::vector<RunSpec *> runs;
  if (!opts.runfile.empty()) { /**< get config from json file */
    RunSpecParse runspecfile(opts.runfile);
    runs = runspecfile.getruns(opts.runspec, opts.basedir, opts.ofile,
                               opts.start, opts.end);
  } else { /**< get config from command line */
    auto dir = opts.basedir + opts.dir;
    runs.push_back(new RunSpec(dir, opts.prefix,
                               opts.postfix, opts.start, opts.end, opts.ofile,
                               -1));
  }

  // Start the analysis
  for (auto run : runs) {
    opts.ofile = run->ofile_;
    Analyze analyze(opts);
    analyze.batchreader(run->dir_, run->prefix_, run->postfix_, run->start_,
                        run->end_);
    analyze.makecal(run->thresh_);
  }
}
