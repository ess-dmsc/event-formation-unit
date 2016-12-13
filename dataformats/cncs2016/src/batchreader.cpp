/** Copyright (C) 2016 European Spallation Source ERIC */

#include <Analyse.h>
#include <Args.h>
#include <RawDataFiles.h>

int main(int argc, char *argv[]) {
  Args opts(argc, argv);
  Analyze analyze(opts);
  analyze.batchreader(opts.dir, opts.prefix, opts.postfix, opts.start,
                      opts.end);
  analyze.makecal();
}
