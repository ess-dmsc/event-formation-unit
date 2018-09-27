/** Copyright (C) 2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Test data for parsing Multi-Blade configuration files
///
//===----------------------------------------------------------------------===//

#include <string>

#pragma once

std::string invalidjson = R"(
  { {{77}}
    "InstrumentGeometry": "Estia"
  }
)";

std::string incompleteconfig = R"(
  {
    "InstrumentGeometry": "Estia"
  }
)";

std::string validconfig = R"(
  {
    "InstrumentGeometry": "Estia",

    "DigitizerConfig" : [
      { "index" : 0, "id" : 137 },
      { "index" : 1, "id" : 143 },
      { "index" : 2, "id" : 142 },
      { "index" : 3, "id" :  31 },
      { "index" : 4, "id" :  33 },
      { "index" : 5, "id" :  34 }
    ],

    "TimeTickNS": 17
  }
)";
