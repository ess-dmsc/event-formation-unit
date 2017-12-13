//
//  ADC_Readout.hpp
//  ADC_Data_Receiver
//
//  Created by Jonas Nilsson on 2017-10-17.
//  Copyright © 2017 European Spallation Source. All rights reserved.
//

#pragma once

struct InData {
  static const int MaxLength = 2048;
  std::uint8_t Data[MaxLength] = {};
  unsigned int Length = 0;
};
