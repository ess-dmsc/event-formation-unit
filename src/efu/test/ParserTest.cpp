/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <algorithm>
#include <common/detector/EFUArgs.h>
#include <cstring>
#include <efu/Parser.h>
#include <memory>
#include <common/testutils/TestBase.h>

#define UNUSED __attribute__((unused))

static int dummy_command(std::vector<std::string>,
                         char*, unsigned int*) {
  return 0;
}

// clang-format off
std::vector<std::string> commands {
  "STAT_GET_COUNT",                 "STAT_GET_COUNT 0",
  "CMD_GET_COUNT",                  "CMD_GET_COUNT 8",
  "STAT_GET 1",                     "STAT_GET  -1",
  "EXIT",                           "<OK>"
};

std::vector<std::string> commands_badargs {
  "STAT_GET_COUNT 1",
  "STAT_GET",
  "CMD_GET_COUNT 1",
  "CMD_GET",
  "CMD_GET 1 2",
  "CMD_GET 0",
  "CMD_GET 9999",
  "VERSION_GET 1",
  "DETECTOR_INFO_GET 1",
  "EXIT 1"
};

// These commands should 'fail' when the detector is not loaded
std::vector<std::string> check_detector_loaded {
  "DETECTOR_INFO_GET",
  "STAT_GET_COUNT",
  "STAT_GET 1"
};

// clang-format on

class TestDetector : public Detector {
public:
  explicit TestDetector(UNUSED BaseSettings settings) : Detector("No StatName", settings) {
    std::cout << "TestDetector" << std::endl;
  };
  ~TestDetector() { std::cout << "~TestDetector" << std::endl; };
};

DetectorFactory<TestDetector> Factory;

class ParserTest : public TestBase {
protected:
  Parser *parser;
  EFUArgs efu_args;
  Statistics stats;
  BaseSettings settings = efu_args.getBaseSettings();
  int keeprunning{1};

  void SetUp() override {
    auto detectorif = Factory.create(settings);
    parser = new Parser(detectorif, stats, keeprunning);
  }

  void TearDown() override { delete parser; }

  static const unsigned int buffer_size = 9000;

  char *inputbuffer[buffer_size];
  unsigned int ibytes, obytes;
  char *outputbuffer[buffer_size];

  char *input = (char *)inputbuffer;
  char *output = (char *)outputbuffer;
};

/** Test cases below */
TEST_F(ParserTest, InputBuffer) {
  auto res = parser->parse(input, 0, output, &obytes);
  ASSERT_EQ(-Parser::EUSIZE, res);
  ASSERT_EQ(strcmp("Error: <BADSIZE>", output), 0);

  input[0] = 'A';
  input[1] = 'B';
  res = parser->parse(input, 1, output, &obytes);
  ASSERT_EQ('A', input[0]);
  ASSERT_EQ('\0', input[1]);
  ASSERT_EQ(strcmp("Error: <BADCMD>", output), 0);
  ASSERT_EQ(-Parser::EBADCMD, res);

  res = parser->parse(input, 2, output, &obytes);
  ASSERT_EQ('A', input[0]);
  ASSERT_EQ('\0', input[1]);
  ASSERT_EQ(strcmp("Error: <BADCMD>", output), 0);
  ASSERT_EQ(-Parser::EBADCMD, res);

  input[0] = 'A';
  input[1] = '\n';
  input[2] = '\0';
  res = parser->parse(input, 3, output, &obytes);
  ASSERT_EQ('A', input[0]);
  ASSERT_EQ('\0', input[1]);
  ASSERT_EQ('\0', input[2]);
  ASSERT_EQ(strcmp("Error: <BADCMD>", output), 0);
  ASSERT_EQ(-Parser::EBADCMD, res);
}

TEST_F(ParserTest, OversizeData) {
  memset(input, 0x41, buffer_size);
  MESSAGE() << "Max buffer size\n";
  auto res = parser->parse(input, buffer_size, output, &obytes);
  ASSERT_EQ('\0', input[buffer_size - 1]);
  ASSERT_EQ(strcmp("Error: <BADCMD>", output), 0);
  ASSERT_EQ(-Parser::EBADCMD, res);

  MESSAGE() << "Max buffer size + 1\n";
  res = parser->parse(input, buffer_size + 1, output, &obytes);
  ASSERT_EQ(-Parser::EOSIZE, res);
  ASSERT_EQ(strcmp("Error: <BADSIZE>", output), 0);
}

TEST_F(ParserTest, NoTokens) {
  memset(input, 0x20, buffer_size);
  MESSAGE() << "Spaces only\n";
  auto res = parser->parse(input, buffer_size, output, &obytes);
  ASSERT_EQ(-Parser::ENOTOKENS, res);
  ASSERT_EQ(strcmp("Error: <BADCMD>", output), 0);
}

TEST_F(ParserTest, ValidCommands) {
  ASSERT_EQ(0, commands.size() & 1);
  for (auto i = 0U; i < commands.size(); i += 2) {
    const char *cmd = commands[i].c_str();
    const char *reply = commands[i + 1].c_str();
    std::memcpy(input, cmd, strlen(cmd));
    MESSAGE() << "Checking command: " << cmd << "\n";
    auto res = parser->parse(input, strlen(cmd), output, &obytes);
    ASSERT_EQ(obytes, strlen(reply));
    ASSERT_EQ(0, strcmp(output, reply));
    ASSERT_EQ(0, res);
  }
}

TEST_F(ParserTest, BadArgsCommands) {
  for (auto cmdstr : commands_badargs) {
    const char *cmd = cmdstr.c_str();
    std::memcpy(input, cmd, strlen(cmd));
    MESSAGE() << "Checking command: " << cmd << "\n";
    auto res = parser->parse(input, strlen(cmd), output, &obytes);
    ASSERT_EQ(strcmp("Error: <BADARGS>", output), 0);
    ASSERT_EQ(-Parser::EBADARGS, res);
  }
}

TEST_F(ParserTest, VersionGet) {
  auto cmd = "VERSION_GET";
  std::memcpy(input, cmd, strlen(cmd));
  MESSAGE() << "Checking command: " << cmd << "\n";
  auto res = parser->parse(input, strlen(cmd), output, &obytes);
  ASSERT_EQ(0, res);
}

TEST_F(ParserTest, ParserClearCommands) {
  auto cmd = "VERSION_GET";
  std::memcpy(input, cmd, strlen(cmd));
  MESSAGE() << "Checking command: " << cmd << "\n";
  auto res = parser->parse(input, strlen(cmd), output, &obytes);
  ASSERT_EQ(0, res);

  parser->clearCommands();
  std::memcpy(input, cmd, strlen(cmd));
  MESSAGE() << "Checking command: " << cmd << "\n";
  res = parser->parse(input, strlen(cmd), output, &obytes);
  ASSERT_EQ(res, -Parser::EBADCMD);
}

TEST_F(ParserTest, DuplicateCommands) {
  int res = parser->registercmd("DUMMY_COMMAND", dummy_command);
  ASSERT_EQ(res, 0);
  res = parser->registercmd("DUMMY_COMMAND", dummy_command);
  ASSERT_EQ(res, -1);
}

TEST_F(ParserTest, NullDetector) {
  int keeprunning{1};
  Statistics stats;
  Parser parser(nullptr, stats, keeprunning); // No detector, no STAT_GET_COUNT command

  const char *cmd = "STAT_GET_COUNT";
  std::memcpy(input, cmd, strlen(cmd));
  int res = parser.parse(input, strlen(cmd), output, &obytes);
  ASSERT_EQ(res, -Parser::EBADCMD);
}

TEST_F(ParserTest, DetInfoGetNoDetectorLoaded) {
  const char *cmd = "DETECTOR_INFO_GET";
  std::memcpy(input, cmd, strlen(cmd));
  int res = parser->parse(input, strlen(cmd), output, &obytes);
  ASSERT_EQ(res, -Parser::OK);
}

TEST_F(ParserTest, DetectorInfo) {
  const char *cmd = "DETECTOR_INFO_GET";
  std::memcpy(input, cmd, strlen(cmd));
  int res = parser->parse(input, strlen(cmd), output, &obytes);
  ASSERT_EQ(res, -Parser::OK);
}

TEST_F(ParserTest, CmdGetCount) {
  const char *cmd = "CMD_GET_COUNT";
  std::memcpy(input, cmd, strlen(cmd));
  int res = parser->parse(input, strlen(cmd), output, &obytes);
  MESSAGE() << output << '\n';
  ASSERT_EQ(res, -Parser::OK);
}

TEST_F(ParserTest, CmdGet) {
  const char *cmd = "CMD_GET 1";
  std::memcpy(input, cmd, strlen(cmd));
  int res = parser->parse(input, strlen(cmd), output, &obytes);
  MESSAGE() << output << '\n';
  ASSERT_EQ(res, -Parser::OK);
}

// TEST_F(ParserTest, ExitCommand) {
//   const char *cmd = "EXIT";
//   std::memcpy(input, cmd, strlen(cmd));
//   int res = parser->parse(input, strlen(cmd), output, &obytes);
//   ASSERT_EQ(res, -Parser::OK);
//   ASSERT_EQ(efu_args->proc_cmd, efu_args->thread_cmd::EXIT);
// }

int main(int argc, char **argv) {
  int __attribute__((unused)) ret = chdir("src");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
