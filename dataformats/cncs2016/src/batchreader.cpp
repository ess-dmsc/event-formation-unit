/** Copyright (C) 2016 European Spallation Source ERIC */

#include <Analyse.h>
#include <Args.h>
#include <RunSpecParse.h>

int main(int argc, char *argv[]) {
  Args opts(argc, argv);

  std::vector<RunSpec *> runs;
  if (!opts.runfile.empty()) { /**< get config from json file */
    RunSpecParse runspecfile(opts.runfile);
    runs = runspecfile.getruns(opts.runspec, opts.basedir, opts.ofile, opts.cfile,
    opts.start, opts.end);
  } else { /**< get config from command line */
    runs.push_back(new RunSpec(opts.basedir + opts.dir, opts.prefix, opts.postfix,
                   opts.start, opts.end, opts.ofile, opts.cfile));
  }

  // Start the analysis
  for (auto run : runs) {
    opts.ofile = run->ofile_;
    opts.cfile = run->cfile_;
    Analyze analyze(opts);
    analyze.batchreader(run->dir_, run->prefix_, run->postfix_, run->start_, run->end_);
    analyze.makecal();
  }
}
