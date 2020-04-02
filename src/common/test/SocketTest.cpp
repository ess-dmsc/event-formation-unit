/** Copyright (C) 2018 European Spallation Source */

#include <src/test/TestBase.h>
#include <common/Socket.h>

std::vector<std::string> ipOk = {"0.0.0.0", "10.10.10.10", "127.0.0.1", "224.1.2.3", "255.255.255.255"};
std::vector<std::string> ipNotOk = {"a.0.0.0", "1.2.3", "1.2", "", "127.0.0.256", "metrics"};

class SocketTest : public ::testing::Test {
protected:
};

TEST_F(SocketTest, ConstructorValid) {
  Socket udpsocket(Socket::SocketType::UDP);
  ASSERT_TRUE(udpsocket.isValidSocket());

  Socket tcpsocket(Socket::SocketType::TCP);
  ASSERT_TRUE(tcpsocket.isValidSocket());
}

TEST_F(SocketTest, SendUninitialized) {
  char buffer[100];
  Socket udpsocket(Socket::SocketType::UDP);
  ASSERT_TRUE(udpsocket.isValidSocket());
  auto res = udpsocket.send(buffer, 100);
  ASSERT_TRUE(res < 0);
  ASSERT_FALSE(udpsocket.isValidSocket());

  Socket tcpsocket(Socket::SocketType::TCP);
  ASSERT_TRUE(tcpsocket.isValidSocket());
  res = tcpsocket.send(buffer, 100);
  ASSERT_TRUE(res < 0);
  ASSERT_FALSE(tcpsocket.isValidSocket());
}

TEST_F(SocketTest, ValidInvalidIp) {
  for (auto ipaddr : ipOk) {
    ASSERT_TRUE(Socket::isValidIp(ipaddr));
    auto res = Socket::getHostByName(ipaddr);
    ASSERT_TRUE(res == ipaddr);
  }
  for (auto ipaddr : ipNotOk) {
    ASSERT_FALSE(Socket::isValidIp(ipaddr));
  }
}

TEST_F(SocketTest, InetAtonInvalidIP) {
  Socket tcpsocket(Socket::SocketType::TCP);
  ASSERT_THROW(tcpsocket.setLocalSocket("invalidipaddress", 9000), std::runtime_error);
  ASSERT_THROW(tcpsocket.setLocalSocket("127.0.0.1", 22), std::runtime_error);
  ASSERT_THROW(tcpsocket.setRemoteSocket("invalidipaddress", 9000), std::runtime_error);
}

TEST_F(SocketTest, PortInUse) {
  Socket tcpsocket(Socket::SocketType::TCP);
  // ssh port is already in use on all platforms
  ASSERT_THROW(tcpsocket.setLocalSocket("127.0.0.1", 22), std::runtime_error);
}


TEST_F(SocketTest, InvalidGetSockOpt) {
  Socket tcpsocket(Socket::SocketType::TCP);
  // force file descriptor (fd) to be set to -1 by failes send()
  auto res = tcpsocket.send(nullptr, 0);
  ASSERT_TRUE(res < 0);
  // Then ask for buffer sizes for invalid fd
  int TxBuffer, RxBuffer;
  tcpsocket.getBufferSizes(TxBuffer, RxBuffer);
  ASSERT_EQ(TxBuffer, -1);
  ASSERT_EQ(RxBuffer, -1);
}

// Create tcp transmitter and send 0 and !=0 number of bytes
// to localhost port 22 (ssh) which should always be active
TEST_F(SocketTest, TCPTransmitter) {
  char DummyData[] {0x01, 0x02, 0x03, 0x04};
  TCPTransmitter Xmitter("127.0.0.1", 22);
  auto res = Xmitter.senddata(DummyData, 0);
  ASSERT_EQ(res, 0);

  res = Xmitter.senddata((const char *)&DummyData, sizeof(DummyData));
  ASSERT_EQ(res, sizeof(DummyData));
}

TEST_F(SocketTest, GetHostByName) {
  std::string name {"localhost"};
  auto res = Socket::getHostByName(name);
  ASSERT_TRUE(res == "127.0.0.1");
  for (auto ipaddr : ipOk) {
     res = Socket::getHostByName(ipaddr);
    ASSERT_TRUE(res == ipaddr);
  }
  // Checking weird case - not sure if this is right
  // this step can be deleted if it causes problems later
  std::string weirdIp {"8.8.8"};
  res = Socket::getHostByName(weirdIp);
  ASSERT_TRUE(res == "8.8.0.8");
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
