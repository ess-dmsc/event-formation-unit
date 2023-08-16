// Copyright (C) 2018 European Spallation Source

#include <common/system/Socket.h>
#include <common/testutils/TestBase.h>
#include <stdio.h>

std::vector<std::string> ipOk = {"0.0.0.0", "10.10.10.10", "127.0.0.1",
                                 "224.1.2.3", "255.255.255.255"};
std::vector<std::string> ipNotOk = {"a.0.0.0", "1.2.3", "1.2",
                                    "", "127.0.0.256", "metrics"};

class SocketTest : public ::testing::Test
{

protected:
  const int TEST_PORT_NUMBER = 8922;

  int socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);

  // SetUp function, executed before the test cases.
  // If there is no service listening on port,
  // TEST_PORT_NUMBER we setup one.
  void SetUp() override
  {
    struct sockaddr_in serv_addr;

    ASSERT_GE(socketFileDescriptor, 0) << "Cannot create test socket during the SetUp() phase of the tests.";

    // Setup local address and port for start listening server
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(TEST_PORT_NUMBER);

    // Try to bind to the socket on local address on port
    // TEST_PORT_NUMBER to create a listening server and block other
    // services to bind to that port. This technique useful
    // in case of containerized test runs. If there is already a
    // process on this port the test not star server.
    if (bind(socketFileDescriptor, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) >= 0)
      listen(socketFileDescriptor, 5);
  }

  // Function execured after test cases to eg. close resources
  void TearDown() override
  {
    close(socketFileDescriptor);
  }
};

TEST_F(SocketTest, ConstructorValid)
{
  Socket udpsocket(Socket::SocketType::UDP);
  ASSERT_TRUE(udpsocket.isValidSocket());

  Socket tcpsocket(Socket::SocketType::TCP);
  ASSERT_TRUE(tcpsocket.isValidSocket());
}

TEST_F(SocketTest, SendUninitialized)
{
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

TEST_F(SocketTest, ValidInvalidIp)
{
  for (auto ipaddr : ipOk)
  {
    ASSERT_TRUE(Socket::isValidIp(ipaddr));
    auto res = Socket::getHostByName(ipaddr);
    ASSERT_TRUE(res == ipaddr);
  }
  for (auto ipaddr : ipNotOk)
  {
    ASSERT_FALSE(Socket::isValidIp(ipaddr));
  }
}

TEST_F(SocketTest, InetAtonInvalidIP)
{
  Socket tcpsocket(Socket::SocketType::TCP);
  ASSERT_THROW(tcpsocket.setLocalSocket("invalidipaddress", 9000),
               std::runtime_error);
  ASSERT_THROW(tcpsocket.setLocalSocket("127.0.0.1", TEST_PORT_NUMBER), std::runtime_error);
  ASSERT_THROW(tcpsocket.setRemoteSocket("invalidipaddress", 9000),
               std::runtime_error);
}

TEST_F(SocketTest, PortInUse)
{
  Socket tcpsocket(Socket::SocketType::TCP);

  ASSERT_THROW(tcpsocket.setLocalSocket("127.0.0.1", TEST_PORT_NUMBER), std::runtime_error);
}

TEST_F(SocketTest, IsMulticast)
{
  ASSERT_TRUE(Socket::isMulticast("224.1.2.3"));
  ASSERT_FALSE(Socket::isMulticast("240.1.2.3"));
}

// Create tcp transmitter and send 0 and !=0 number of bytes
// to localhost port TEST_PORT_NUMBER where we set or listening
// service during the setup phase.
TEST_F(SocketTest, TCPTransmitter)
{
  char DummyData[]{0x01, 0x02, 0x03, 0x04};
  TCPTransmitter Xmitter("127.0.0.1", TEST_PORT_NUMBER);
  auto res = Xmitter.senddata(DummyData, 0);
  ASSERT_EQ(res, 0);

  res = Xmitter.senddata((const char *)&DummyData, sizeof(DummyData));
  ASSERT_EQ(res, sizeof(DummyData));
}

TEST_F(SocketTest, UDPTransmitter)
{
  Socket::Endpoint local("127.0.0.1", 13241);
  Socket::Endpoint remote("127.0.0.1", 13241);
  UDPTransmitter UDPXmitter(local, remote);

  ASSERT_EQ(UDPXmitter.isValidSocket(), true);
}

TEST_F(SocketTest, GetHostByName)
{
  std::string name{"localhost"};
  auto res = Socket::getHostByName(name);
  ASSERT_TRUE(res == "127.0.0.1");
  for (auto ipaddr : ipOk)
  {
    res = Socket::getHostByName(ipaddr);
    ASSERT_TRUE(res == ipaddr);
  }
  // Checking weird case - not sure if this is right
  // this step can be deleted if it causes problems later
  std::string weirdIp{"8.8.8"};
  res = Socket::getHostByName(weirdIp);
  ASSERT_TRUE(res == "8.8.0.8");
}

TEST_F(SocketTest, GetHostByNameInvalid)
{
  Socket tcpsocket(Socket::SocketType::TCP);
  std::string InvalidHostName("#$%@^");
  ASSERT_THROW(tcpsocket.getHostByName(InvalidHostName), std::runtime_error);
}

TEST_F(SocketTest, MultiCastSetTTL)
{
  Socket udpsocket(Socket::SocketType::UDP);
  ASSERT_TRUE(udpsocket.isValidSocket());
  udpsocket.setMulticastTTL();
}

TEST_F(SocketTest, MultiCastSetReceive)
{
  Socket udpsocket(Socket::SocketType::UDP);
  ASSERT_TRUE(udpsocket.isValidSocket());
  ASSERT_NO_THROW(udpsocket.setLocalSocket("224.1.2.1", 9729));
}

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
