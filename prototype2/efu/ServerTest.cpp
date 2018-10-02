/** Copyright (C) 2018 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <chrono>
#include <efu/Server.h>
#include <sys/socket.h>
#include <test/TestBase.h>
#include <thread>

uint16_t ServerPort = 8888;

/// Used in pthread to connect to server and send data
void senddata() {
  struct sockaddr_in server;
  int sock = socket(AF_INET , SOCK_STREAM , 0);
  if (sock == -1) {
      printf("Could not create socket");
      return;
  }

  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons( ServerPort );

  //Connect to remote server
  if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
      perror("connect failed. Error");
      return;
  }

  const char * message = "DETECTOR_INFO_GET\n";
  if( send(sock , message , strlen(message) , 0) < 0)
  {
      puts("Send failed");
      return;
  }

  /// Allow time for test to poll for data
  std::this_thread::sleep_for(std::chrono::seconds(2));
  close(sock);
}

class TestDetector : public Detector {
public:
  TestDetector(BaseSettings settings) : Detector("No name", settings) {
    std::cout << "TestDetector" << std::endl;
  };
  ~TestDetector() { std::cout << "~TestDetector" << std::endl; };
};

DetectorFactory<TestDetector> Factory;

// clang-format on

class ServerTest : public TestBase {
protected:

  int keep_running = 1;
  EFUArgs efu_args;
  BaseSettings settings = efu_args.getBaseSettings();
  Parser * parser;
  virtual void SetUp() {
      auto detectorif = Factory.create(settings);
      parser = new Parser(detectorif, keep_running);
  }

  virtual void TearDown() { }
};

/** Test cases below */
TEST_F(ServerTest, Constructor) {
  Server server(ServerPort, *parser);
  ASSERT_TRUE(server.getServerFd() != -1);
  ASSERT_TRUE(server.getServerPort() == ServerPort);
  ASSERT_EQ(server.getNumClients(), 0);
}

TEST_F(ServerTest, ServerSendInvalidFd) {
  Server server(ServerPort, *parser);
  ASSERT_TRUE(server.getServerFd() != -1);
  ASSERT_TRUE(server.getServerPort() == ServerPort);
  ASSERT_EQ(server.getNumClients(), 0);
  ASSERT_EQ(server.serverSend(42), -1);
}

TEST_F(ServerTest, PollNoData) {
  Server server(ServerPort, *parser);
  server.serverPoll();
  ASSERT_EQ(server.getNumClients(), 0);
  ASSERT_TRUE(server.getServerFd() != -1);
  ASSERT_TRUE(server.getServerPort() == ServerPort);
}

TEST_F(ServerTest, PollWithData) {
  Server server(ServerPort, *parser);
  std::thread sendthread(senddata);
  server.serverPoll();
  server.serverPoll();
  server.serverPoll();
  ASSERT_EQ(server.getNumClients(), 1);
  ASSERT_TRUE(server.getServerFd() != -1);
  ASSERT_TRUE(server.getServerPort() == ServerPort);

  sendthread.join();
  server.serverPoll();
  server.serverPoll();
  server.serverPoll();
  ASSERT_EQ(server.getNumClients(), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
