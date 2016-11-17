/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Command parser
 */

#include <common/EFUArgs.h>

class Parser {
  public:
    /** @todo document
     */
    Parser(EFUArgs & args) : opts(args) {}

    /** @todo document
     */
    int parse(char * input, unsigned int isize, char * output, unsigned int * osize);

  private:
    EFUArgs & opts;
};
