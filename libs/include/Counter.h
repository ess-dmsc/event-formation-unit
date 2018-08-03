/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
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
  /// count up by
  void add() { n++; }

  /// return number of adds
  long long count(void) { return n; }

  void clear() { n = 0; }
};
