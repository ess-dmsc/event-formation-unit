// Copyright (C) 2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Multi-Blade reference data from DG AMOR@PSI measurements
///
/// If the reference data directory is detected HAS_REFDATA will be defined.
/// Then if INCLUDE_LARGE_DATA is defined the external source files will be
/// included.
/// This increases compile time significantly so default is not to include them.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <cstdio>
#include <vector>

// #define INCLUDE_DS1
// #define INCLUDE_DS1_FILTERED
// #define INCLUDE_DS2

struct MBHits {
  float Time;
  float Channel;
  float AdcValue;

  void print() {
    printf("t: %10.7f, c: %u, a: %u\n", Time, (uint16_t)Channel,
           (uint16_t)AdcValue);
  }
};

// As we get the data from FP (from mail correspondence)
struct MBEvents {
  float y;       // Wire coordinate (== channel) (for MB18Freia)
  float x;       // Strip coordinate (== channels - 32)
  float time;    // Time (s)
  float unused2; // Sum of wire adc
  float unused3; // Sum of strip adc
  float unused4; // Wire multiplicity
  float unused5; // Strip multiplicity
};

extern std::vector<struct MBHits> FPRefData;

extern std::vector<struct MBHits> DS2S_ST_FF;
extern std::vector<struct MBEvents> DS2S_ST_FF_Res;

extern std::vector<struct MBHits> DS1L_SF_FF;
extern std::vector<struct MBEvents> DS1L_ST_FF_Res;

extern std::vector<struct MBHits> DS1L_SF_FT;
extern std::vector<struct MBEvents> DS1L_ST_FT_Res;

extern std::vector<struct MBHits> DS2L_SF_FF;
extern std::vector<struct MBEvents> DS2L_ST_FF_Res;
