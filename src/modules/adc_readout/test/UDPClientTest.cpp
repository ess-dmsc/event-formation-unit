/** Copyright (C) 2018-2020 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <adc_readout/UDPClient.h>
#include "TestUDPServer.h"
#include <chrono>
#include <gtest/gtest.h>

using namespace std::chrono_literals;

class UDPClientTest : public ::testing::Test {
public:
  void SetUp() override { Service = std::make_shared<asio::io_service>(); }
  void TearDown() override {}
  std::shared_ptr<asio::io_service> Service;
};

TEST_F(UDPClientTest, SingleUDPPacket) {
  int BytesToTransmit = 1470;
  std::uint16_t ListenOnPort = GetPortNumber();
  auto SendToPort = ListenOnPort;
  TestUDPServer Server(GetPortNumber(), SendToPort, BytesToTransmit);
  int BytesReceived = 0;
  int PacketsHandled = 0;
  std::function<void(InData const &Packet)> PacketHandler =
      [&BytesReceived, &PacketsHandled](auto &Packet) {
        BytesReceived += Packet.Length;
        ++PacketsHandled;
      };
  UDPClient TestClient(Service, "0.0.0.0", ListenOnPort, PacketHandler);
  Server.startPacketTransmission(1, 0);
  Service->run_for(100ms);
  EXPECT_EQ(BytesReceived, BytesToTransmit);
  EXPECT_EQ(PacketsHandled, 1);
}

TEST_F(UDPClientTest, MultipleUDPPackets) {
  int BytesToTransmit = 1470;
  std::uint16_t ListenOnPort = GetPortNumber();
  auto SendToPort = ListenOnPort;
  TestUDPServer Server(GetPortNumber(), SendToPort, BytesToTransmit);
  int BytesReceived = 0;
  int PacketsHandled = 0;
  std::function<void(InData const &Packet)> PacketHandler =
      [&BytesReceived, &PacketsHandled](auto &Packet) {
        BytesReceived += Packet.Length;
        ++PacketsHandled;
      };
  auto NrOfPackets = 5;
  UDPClient TestClient(Service, "0.0.0.0", ListenOnPort, PacketHandler);
  Server.startPacketTransmission(NrOfPackets, 5);
  Service->run_for(100ms);
  EXPECT_EQ(BytesReceived, BytesToTransmit * NrOfPackets);
  EXPECT_EQ(PacketsHandled, NrOfPackets);
}

class UDPClientStandIn : UDPClient {
public:
  UDPClientStandIn(std::shared_ptr<asio::io_service> const &IOService,
                   std::string const &Interface, std::uint16_t Port,
                   std::function<void(InData const &Packet)> Handler)
      : UDPClient(IOService, Interface, Port, std::move(Handler)){};
  using UDPClient::Socket;
};

TEST_F(UDPClientTest, PortInUseError) {
  int BytesToTransmit = 1470;
  std::uint16_t ListenOnPort = GetPortNumber();
  auto SendToPort = ListenOnPort;
  TestUDPServer Server(GetPortNumber(), SendToPort, BytesToTransmit);
  int BytesReceived = 0;
  int PacketsHandled = 0;
  std::function<void(InData const &Packet)> PacketHandler =
      [&BytesReceived, &PacketsHandled](auto &Packet) {
        BytesReceived += Packet.Length;
        ++PacketsHandled;
      };
  UDPClientStandIn TestClient1(Service, "0.0.0.0", ListenOnPort, PacketHandler);
  Service->run_for(100ms);
  EXPECT_TRUE(TestClient1.Socket.is_open());
  UDPClientStandIn TestClient2(Service, "0.0.0.0", ListenOnPort, PacketHandler);
  Service->run_for(100ms);
  EXPECT_FALSE(TestClient2.Socket.is_open());
}

TEST_F(UDPClientTest, MultipleUDPPacketContent) {
  auto NrOfValues = 100u;
  std::vector<std::uint8_t> TestData(NrOfValues);
  for (std::size_t i = 0; i < TestData.size(); i++) {
    TestData[i] = static_cast<std::uint8_t>(i);
  }
  auto BytesToTransmit = NrOfValues * sizeof(TestData[0]);
  std::uint16_t ListenOnPort = GetPortNumber();
  auto SendToPort = ListenOnPort;
  TestUDPServer Server(GetPortNumber(), SendToPort, &TestData[0],
                       BytesToTransmit);
  auto BytesReceived = 0u;
  auto PacketsHandled = 0u;
  std::function<void(InData const &Packet)> PacketHandler =
      [&BytesReceived, &PacketsHandled, &TestData](auto &Packet) {
        EXPECT_EQ(std::memcmp(&Packet.Data[0], &TestData[0], Packet.Length), 0);
        BytesReceived += Packet.Length;
        ++PacketsHandled;
      };
  auto NrOfPackets = 5u;
  UDPClient TestClient(Service, "0.0.0.0", ListenOnPort, PacketHandler);
  Server.startPacketTransmission(NrOfPackets, 5);
  Service->run_for(100ms);
  EXPECT_EQ(BytesReceived, BytesToTransmit * NrOfPackets);
  EXPECT_EQ(PacketsHandled, NrOfPackets);
}
