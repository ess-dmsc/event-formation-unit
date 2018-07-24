/// Copyright (C) 2016-2018 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief a simple counter class
/// \todo check if this is actually in use, and where
///
//===----------------------------------------------------------------------===//

#pragma once
class Counter {

private:
  long long n{0};

public:
  /** count up by 1*/
  void add() { n++; }

  /** return number of adds */
  long long count(void) { return n; }

  void clear() { n = 0; }
};
