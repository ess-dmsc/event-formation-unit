/** Copyright (C) 2018 European Spallation Source ERIC */

#include <efu/Server.h>
#include <test/TestBase.h>


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
  uint16_t ServerPort = 8888;
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
}

TEST_F(ServerTest, Poll) {
  Server server(ServerPort, *parser);
  server.serverPoll();
  ASSERT_TRUE(server.getServerFd() != -1);
  ASSERT_TRUE(server.getServerPort() == ServerPort);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
