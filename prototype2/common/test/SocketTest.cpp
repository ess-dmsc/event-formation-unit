/** Copyright (C) 2018 European Spallation Source */

#include <prototype2/test/TestBase.h>
#include <common/Socket.h>


class SocketTest : public ::testing::Test {
protected:
};

TEST_F(SocketTest, ConstructorValid) {
  Socket udpsocket(Socket::type::UDP);
  ASSERT_TRUE(udpsocket.isValidSocket());

  Socket tcpsocket(Socket::type::TCP);
  ASSERT_TRUE(tcpsocket.isValidSocket());
}

TEST_F(SocketTest, SendUninitialized) {
  char buffer[100];
  Socket udpsocket(Socket::type::UDP);
  ASSERT_TRUE(udpsocket.isValidSocket());
  auto res = udpsocket.send(buffer, 100);
  ASSERT_TRUE(res < 0);
  ASSERT_FALSE(udpsocket.isValidSocket());

  Socket tcpsocket(Socket::type::TCP);
  ASSERT_TRUE(tcpsocket.isValidSocket());
  res = tcpsocket.send(buffer, 100);
  ASSERT_TRUE(res < 0);
  ASSERT_FALSE(tcpsocket.isValidSocket());
}

TEST_F(SocketTest, ValidInvalidIp) {
  std::vector<std::string> ipOk = {"0.0.0.0", "10.10.10.10", "127.0.0.1", "224.1.2.3", "255.255.255.255"};
  std::vector<std::string> ipNotOk = {"a.0.0.0", "1.2.3", "1.2", "", "127.0.0.256", "metrics"};

  for (auto ipaddr : ipOk) {
    ASSERT_TRUE(Socket::isValidIp(ipaddr));
    auto res = Socket::getHostByName(ipaddr);
    ASSERT_TRUE(res == ipaddr);
  }
  for (auto ipaddr : ipNotOk) {
    ASSERT_FALSE(Socket::isValidIp(ipaddr));
  }
}

TEST_F(SocketTest, GetHostByName) {
  std::string name {"localhost"};
  auto res = Socket::getHostByName(name);
  ASSERT_TRUE(res == "127.0.0.1");
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
