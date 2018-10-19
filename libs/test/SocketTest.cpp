/** Copyright (C) 2018 European Spallation Source */

#include <prototype2/test/TestBase.h>
#include <libs/include/Socket.h>


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

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
