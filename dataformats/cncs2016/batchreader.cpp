/** Copyright (C) 2016 European Spallation Source ERIC */

#include <Analyse.h>
#include <Args.h>

int main(int argc, char *argv[]) {
  Args opts(argc, argv);
  Analyze analyze(opts.ofile);
  analyze.batchreader(opts.dir, opts.prefix, opts.postfix, opts.start,
                      opts.end);
}
