/** Copyright (C) 2018 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <chrono>
#include <efu/Server.h>
#include <sys/socket.h>
#include <test/TestBase.h>
#include <thread>

uint16_t ServerPort = 8889;

constexpr int mask_close = 0x0001;
constexpr int mask_connect = 0x0002;
constexpr int mask_send = 0x0004;
const char * message = "DETECTOR_INFO_GET\n";

/// Used in pthread to connect to server and send data
void client_thread(int command) {
  struct sockaddr_in server;

  int sock = socket(AF_INET , SOCK_STREAM , 0);
  if (sock == -1) {
      printf("Could not create socket\n");
  }

  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons( ServerPort );

  //Connect to remote server
  if (command & mask_connect) {
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("connect failed. Error\n");
    }
  }

  if (command & mask_send) {
    if( send(sock , message , strlen(message) , 0) < 0) {
        printf("Send failed\n");
    }
    /// Allow time for test to poll for data
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }

  if (command & mask_close) {
    close(sock);
  }
}

class TestDetector : public Detector {
public:
  explicit TestDetector(BaseSettings settings) : Detector("No name", settings) { };
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
  Stats stats;
  void SetUp() override {
      auto detectorif = Factory.create(settings);
      parser = new Parser(detectorif, stats, keep_running);
  }

  void TearDown() override {
    delete parser;
  }
};

/** Test cases below */
TEST_F(ServerTest, Constructor) {
  Server server(ServerPort, *parser);
  ASSERT_TRUE(server.getServerFd() != -1);
  ASSERT_TRUE(server.getServerPort() == ServerPort);
  ASSERT_EQ(server.getNumClients(), 0);
}

// TEST_F(ServerTest, ServerSendInvalidFd) {
//   Server server(ServerPort, *parser);
//   ASSERT_EQ(server.getNumClients(), 0);
//   ASSERT_EQ(server.serverSend(42), -1);
// }

TEST_F(ServerTest, PollNoData) {
  Server server(ServerPort, *parser);
  server.serverPoll();
  ASSERT_EQ(server.getNumClients(), 0);
}

TEST_F(ServerTest, PollConnect) {
  Server server(ServerPort, *parser);
  std::thread client(client_thread, mask_connect);
  client.join();
  server.serverPoll();
  server.serverPoll();
  ASSERT_EQ(server.getNumClients(), 1);
  ASSERT_EQ(server.getTotalBytesReceived(), 0);
}

TEST_F(ServerTest, PollWData) {
  Server server(ServerPort, *parser);
  std::thread client(client_thread, mask_connect | mask_send);
  client.join();
  server.serverPoll();
  server.serverPoll();
  ASSERT_EQ(server.getNumClients(), 1);
  ASSERT_EQ(server.getTotalBytesReceived(), strlen(message));
}

TEST_F(ServerTest, PollWDataClose) {
  Server server(ServerPort, *parser);
  std::thread client(client_thread, mask_connect | mask_send | mask_close);
  client.join();
  server.serverPoll();
  server.serverPoll();
  server.serverPoll();
  ASSERT_EQ(server.getNumClients(), 0);
  ASSERT_EQ(server.getTotalBytesReceived(), strlen(message));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
