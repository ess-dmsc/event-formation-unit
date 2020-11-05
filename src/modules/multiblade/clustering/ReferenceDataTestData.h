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

struct MBHits {
  float Time;
  float Channel;
  float AdcValue;

  void print() {
    printf("t: %10.7f, c: %u, a: %u\n", Time, (uint16_t)Channel, (uint16_t)AdcValue);
  }
};

struct MBEvents {
  float y;
  float x;
  float time;
  float unused2;
  float unused3;
  float unused4;
  float unused5;
};

// Small reference dataset from Francesco Oct 2020
// 33 readouts, 10 events, 3 unmatched clusters
std::vector<struct MBHits> FPRefData {
  // Time (s)        Channel           ADC
  {9.1252640e-03, 2.7e+01, 2.1375e+04},
  {9.1254240e-03, 2.8e+01, 1.0315e+04},
  {9.1256320e-03, 5.0e+01, 5.8390e+03},
  {9.1258400e-03, 4.9e+01, 3.2740e+03},
  {9.1260960e-03, 5.1e+01, 1.9790e+03},
  {9.1588000e-03, 1.0e+01, 3.3407e+04},
  {9.1591200e-03, 5.2e+01, 8.6440e+03},
  {9.1593760e-03, 5.1e+01, 4.4420e+03},
  {9.1596320e-03, 5.3e+01, 2.4130e+03},
  {9.2895680e-03, 1.8e+01, 2.5456e+04},
  {9.2901120e-03, 6.2e+01, 4.1300e+03},
  {9.2902560e-03, 6.1e+01, 3.1590e+03},
  {9.4956480e-03, 1.0e+01, 2.4285e+04},
  {9.4960000e-03, 6.0e+01, 6.7930e+03},
  {9.4963840e-03, 5.9e+01, 2.9680e+03},
  {9.6056640e-03, 1.0e+00, 6.0840e+03},
  {9.7041760e-03, 0.0e+00, 1.1635e+04},
  {9.9865280e-03, 4.2e+01, 1.7840e+03},
  {1.0124720e-02, 2.4e+01, 1.9147e+04},
  {1.0125296e-02, 4.6e+01, 3.1150e+03},
  {1.0217376e-02, 2.4e+01, 2.1500e+04},
  {1.0218272e-02, 6.3e+01, 2.1770e+03},
  {1.0253808e-02, 2.5e+01, 1.4624e+04},
  {1.0254560e-02, 6.1e+01, 2.0910e+03},
  {1.0285024e-02, 1.3e+01, 2.7029e+04},
  {1.0285440e-02, 4.2e+01, 5.6720e+03},
  {1.0285872e-02, 4.1e+01, 2.4110e+03},
  {1.0286032e-02, 4.3e+01, 1.6460e+03},
  {1.0379984e-02, 1.4e+01, 3.2236e+04},
  {1.0380480e-02, 3.3e+01, 4.7520e+03},
  {1.0380640e-02, 3.4e+01, 3.1970e+03},
  {1.0454736e-02, 7.0e+00, 1.1311e+04},
  {1.0455744e-02, 3.2e+01, 1.5170e+03}
};


std::vector<struct MBHits> DS2S_ST_FF {
  {2.1077632e-02, 1.1e+01, 2.1548e+04},
  {2.1077920e-02, 4.2e+01, 6.8710e+03},
  {2.1078176e-02, 4.3e+01, 3.7770e+03},
  {2.1078960e-02, 4.1e+01, 6.7800e+02},
  {2.2498512e-02, 5.0e+00, 2.8075e+04},
  {2.2498944e-02, 4.4e+01, 5.2690e+03},
  {2.2499056e-02, 4.5e+01, 3.1070e+03},
  {2.2766480e-02, 2.9e+01, 1.7059e+04},
  {2.2767072e-02, 4.5e+01, 2.9930e+03},
  {2.3929952e-02, 6.0e+00, 3.3579e+04},
  {2.3930352e-02, 4.5e+01, 6.1020e+03},
  {2.3930464e-02, 4.6e+01, 4.7780e+03},
  {2.4011136e-02, 5.0e+00, 9.5980e+03},
  {2.4011904e-02, 4.2e+01, 1.3670e+03},
  {2.5727728e-02, 2.3e+01, 1.7122e+04},
  {2.5728464e-02, 5.0e+01, 2.6510e+03},
  {2.6848656e-02, 9.0e+00, 1.1513e+04},
  {2.6849744e-02, 4.4e+01, 9.7900e+02},
  {2.6961408e-02, 7.0e+00, 1.0931e+04},
  {2.6962496e-02, 4.6e+01, 9.2400e+02},
  {3.4182368e-02, 1.0e+01, 2.0345e+04},
  {3.4182960e-02, 5.0e+01, 3.4160e+03},
  {3.4183344e-02, 4.9e+01, 1.5620e+03},
  {3.9423904e-02, 1.5e+01, 6.9060e+03},
  {3.9481104e-02, 1.0e+01, 2.6541e+04},
  {3.9481504e-02, 4.8e+01, 5.1480e+03},
  {3.9481936e-02, 4.9e+01, 2.1570e+03},
  {4.1635056e-02, 2.7e+01, 1.1110e+04},
  {4.9476944e-02, 2.4e+01, 2.4546e+04}
};

std::vector<struct MBEvents> DS2S_ST_FF_Res {
  {1.1e+01, 1.027e+01, 2.107800e-02, 2.1548e+04, 1.1326e+04, 1.0e+00, 3.0e+00},
  {5.0e+00, 1.237e+01, 2.249900e-02, 2.8075e+04, 8.3760e+03, 1.0e+00, 2.0e+00},
  {2.9e+01, 1.300e+01, 2.276600e-02, 1.7059e+04, 2.9930e+03, 1.0e+00, 1.0e+00},
  {6.0e+00, 1.344e+01, 2.393000e-02, 3.3579e+04, 1.0880e+04, 1.0e+00, 2.0e+00},
  {5.0e+00, 1.000e+01, 2.401100e-02, 9.5980e+03, 1.3670e+03, 1.0e+00, 1.0e+00},
  {2.3e+01, 1.800e+01, 2.572800e-02, 1.7122e+04, 2.6510e+03, 1.0e+00, 1.0e+00},
  {9.0e+00, 1.200e+01, 2.684900e-02, 1.1513e+04, 9.7900e+02, 1.0e+00, 1.0e+00},
  {7.0e+00, 1.400e+01, 2.696100e-02, 1.0931e+04, 9.2400e+02, 1.0e+00, 1.0e+00},
  {1.0e+01, 1.769e+01, 3.418200e-02, 2.0345e+04, 4.9780e+03, 1.0e+00, 2.0e+00},
  {1.0e+01, 1.630e+01, 3.948100e-02, 2.6541e+04, 7.3050e+03, 1.0e+00, 2.0e+00}

};

#ifdef HAS_REFDATA

#ifdef INCLUDE_DS1_SORTED
// Large dataset1 sorted but unfiltered
// Sorted, not filtered
// std::vector<struct MBHits> DS1L_ST_FF {
// #include <dataset1_large_ST_FF_Input.txt>
// };
std::vector<struct MBEvents> DS1L_ST_FF_Res {
#include <dataset1_large_ST_FF_Clustered.txt>
};
#endif // DS1_SORTED

#ifdef INCLUDE_DS1_UNSORTED
// Not sorted, not filtered
std::vector<struct MBHits> DS1L_SF_FF {
#include <dataset1_large_SF_FF_Input.txt>
};
// std::vector<struct MBEvents> DS1L_SF_FF_Res {
// #include <dataset1_large_SF_FF_Clustered.txt>
// };
#endif // DS1_UNSORTED


#ifdef INCLUDE_DS2_SORTED
// Large dataset 2
// Not sorted, not filtered
// std::vector<struct MBHits> DS2L_ST_FF {
// #include <dataset2_large_ST_FF_Input.txt>
// };
std::vector<struct MBEvents> DS2L_ST_FF_Res {
#include <dataset2_large_ST_FF_Clustered.txt>
};
#endif // DS2_SORTED

#ifdef INCLUDE_DS2_UNSORTED
std::vector<struct MBHits> DS2L_SF_FF {
#include <dataset2_large_SF_FF_Input.txt>
};
// std::vector<struct MBEvents> DS2L_SF_FF_Res {
// #include <dataset2_large_SF_FF_Clustered.txt>
// };
#endif // DS2_UNSORTED

#endif // HAS_REFDATA
