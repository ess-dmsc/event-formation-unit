#include <iostream>
#include <fstream>
#include "UDPServer.h"

int main(int argc, char **argv) {
  std::ifstream InStream("DataPacketTemplate.dat", std::ios::binary);
  if (not InStream.good()) {
    std::cout << "Stream is not good, exiting." << std::endl;
    return 0;
  }
  
  InStream.seekg(0, std::ios::end);
  std::size_t FileSize = InStream.tellg();
  InStream.seekg(0, std::ios::beg);
  std::unique_ptr<std::uint8_t[]> TemplatePacketData(new std::uint8_t[FileSize]);
  InStream.read(reinterpret_cast<char*>(TemplatePacketData.get()), FileSize);
  UDPServer server(2048, 9000);
  std::uint8_t TestData[150];
  server.TransmitPacket(TestData, 150);
  while (not server.IsOk()) {
  }
  for (int i = 0; i < 1000000; ++i) {
    server.TransmitPacket(TemplatePacketData.get(), FileSize);
  }
  std::cout << "Waiting for transmit thread to exit!" << std::endl;
  return 0;
}
