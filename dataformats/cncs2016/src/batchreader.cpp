/** Copyright (C) 2016 European Spallation Source ERIC */

#include <Analyse.h>
#include <Args.h>
#include <RawDataFiles.h>

int main(int argc, char *argv[]) {
  Args opts(argc, argv);
  if (1) { /**< @todo add cmd line option */
    for (auto run : filelist) {
      if (run.index < 21) {
        continue;
      }
      std::string root("/home/morten/nfs/data/raw/MG_CNCS/");
      std::string odir("output/");
      opts.dir = root + run.sub_dir;
      opts.prefix = run.file_prefix;
      opts.postfix = run.file_postfix;
      opts.start = run.range_start;
      opts.end = run.range_end;
      opts.ofile = odir + run.file_prefix;
      opts.cfile = odir + run.file_prefix;

      Analyze analyze(opts);
      analyze.batchreader(opts.dir, opts.prefix, opts.postfix, opts.start, opts.end);
      analyze.makecal();
    }
  }
}
