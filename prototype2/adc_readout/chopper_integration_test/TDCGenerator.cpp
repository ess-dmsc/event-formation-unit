//
// Created by Jonas Nilsson on 2017-11-08.
//

#include "TDCGenerator.h"

namespace TDC {

TDCGenerator::TDCGenerator(std::string Name) : SourceName(Name) {
  
}

PacketInfo TDCGenerator::GetTDCPacket(double TimeStamp) {
  Builder->Clear();
  auto PVNameString = Builder->CreateString(SourceName);
  DoubleBuilder ValueStruct(*Builder.get());
  ValueStruct.add_value(TimeStamp);
  auto ValueOffset = ValueStruct.Finish();
  LogDataBuilder StructureBuilder(*Builder.get());
  StructureBuilder.add_source_name(PVNameString);
  StructureBuilder.add_value(ValueOffset.Union());
  StructureBuilder.add_value_type(Value::Value_Double);
  StructureBuilder.add_timestamp(TimeStamp * 1e9 + 0.5);
  Builder->Finish(StructureBuilder.Finish(), LogDataIdentifier());
  return PacketInfo{Builder->GetBufferPointer(), Builder->GetSize()};
}

}
