/** Copyright (C) 2016 European Spallation Source */

#pragma once
class Counter {

private:
  long long n{0};

public:
  /** count up by 1*/
  void add() { n++; }

  /** return number of adds */
  long long count(void) {return n; }

  void clear() { n = 0; }
};
