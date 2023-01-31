// Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia geometry class
///
/// Mapping from digital identifiers to x- and y- coordinates
//===----------------------------------------------------------------------===//

#include <stdint.h>
#include <modules/freia/geometry/FreiaGeometry.h>


namespace VMM{
  uint8_t FreiaGeometry::getPlane(const ESSReadout::VMM3Parser::VMM3Data& Data){
    if (xCoord(Data.VMM, Data.Channel)){
      return 0;
    }
    else{
      return 1;
    }
  }

   uint16_t FreiaGeometry::getPixel(const ESSReadout::VMM3Parser::VMM3Data& Data){
    if (xCoord(Data.VMM, Data.Channel)){
      return xCoord(Data.VMM, Data.Channel);
    }
    else{
      return yCoord(0, Data.VMM, Data.Channel);
    }
  }
}