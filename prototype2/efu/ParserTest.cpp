/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <algorithm>
#include <common/EFUArgs.h>
#include <cstring>
#include <efu/Parser.h>
#include <memory>
#include <test/TestBase.h>

using namespace std;

extern int forcefstatfail;
extern int forcereadfail;

#define UNUSED __attribute__((unused))

static int dummy_command(std::vector<std::string> UNUSED cmdargs,
                         char UNUSED *output, unsigned int UNUSED *obytes) {
  return 0;
}

// clang-format off
std::vector<std::string> commands {
  // doesnt work when tests are called outside prototype2/ dir
  "CSPEC_LOAD_CALIB calib_data/cal_zero", "<OK>",
  "CSPEC_SHOW_CALIB",               "wire 0 0x0000, grid 0 0x0000",
  "CSPEC_SHOW_CALIB 5",             "wire 5 0x0000, grid 5 0x0000",
  "STAT_GET_COUNT",                 "STAT_GET_COUNT 0",
  "STAT_GET 1",                     "STAT_GET  -1"
};

std::vector<std::string> commands_badargs {
  "CSPEC_LOAD_CALIB",
  "CSPEC_LOAD_CALIB file1 file2",
  "CSPEC_LOAD_CALIB file_not_exist",
  "CSPEC_LOAD_CALIB calib_data/cal_badsize",
  "CSPEC_LOAD_CALIB calib_data/nogcal",
  "CSPEC_SHOW_CALIB 1 2",
  "CSPEC_SHOW_CALIB 16384",
  "STAT_GET_COUNT 1",
  "STAT_GET",
  "VERSION_GET 1",
  "DETECTOR_INFO_GET 1"
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
  TestDetector(UNUSED void *args) { cout << "TestDetector" << endl; };
  ~TestDetector() { cout << "~TestDetector" << endl; };
};

class TestDetectorFactory : public DetectorFactory {
public:
  std::shared_ptr<Detector> create(void *args) {
    cout << "TestDetectorFactory" << endl;
    return std::shared_ptr<Detector>(new TestDetector(args));
  }
};

TestDetectorFactory Factory;

class ParserTest : public TestBase {
protected:
  Parser *parser;

  virtual void SetUp() {
    parser = new Parser();
    efu_args = new EFUArgs(0, NULL);
    efu_args->detectorif = Factory.create(0);
  }

  virtual void TearDown() {
    delete parser;
    delete efu_args;
  }

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

TEST_F(ParserTest, DuplicateCommands) {
  int res = parser->registercmd("DUMMY_COMMAND", dummy_command);
  ASSERT_EQ(res, 0);
  res = parser->registercmd("DUMMY_COMMAND", dummy_command);
  ASSERT_EQ(res, -1);
}

TEST_F(ParserTest, SysCallFail) {
  const char *cmd = "CSPEC_LOAD_CALIB calib_data/cal_zero";
  std::memcpy(input, cmd, strlen(cmd));
  forcefstatfail = 1;
  int res = parser->parse(input, strlen(cmd), output, &obytes);
  ASSERT_EQ(res, -Parser::EBADARGS);
  ASSERT_EQ(0, forcefstatfail);

  forcereadfail = 1;
  std::memcpy(input, cmd, strlen(cmd));
  res = parser->parse(input, strlen(cmd), output, &obytes);
  ASSERT_EQ(res, -Parser::EBADARGS);
  ASSERT_EQ(0, forcereadfail);
}

TEST_F(ParserTest, DetInfoGetNoDetectorLoaded) {
  efu_args->detectorif = 0;
  const char *cmd = "DETECTOR_INFO_GET";
  std::memcpy(input, cmd, strlen(cmd));
  int res = parser->parse(input, strlen(cmd), output, &obytes);
  ASSERT_EQ(res, -Parser::OK);
}

TEST_F(ParserTest, StatGetCountNoDetectorLoaded) {
  efu_args->detectorif = 0;
  for (auto cmdstr : check_detector_loaded) {
    MESSAGE() << cmdstr << "\n";
    const char *cmd = cmdstr.c_str();
    std::memcpy(input, cmd, strlen(cmd));
    int res = parser->parse(input, strlen(cmd), output, &obytes);
    ASSERT_EQ(res, -Parser::OK);
    ASSERT_NE(strstr(output, "error: no detector loaded"), nullptr);
  }
}

TEST_F(ParserTest, DetectorInfo) {
  const char *cmd = "DETECTOR_INFO_GET";
  std::memcpy(input, cmd, strlen(cmd));
  int res = parser->parse(input, strlen(cmd), output, &obytes);
  ASSERT_EQ(res, -Parser::OK);
}

int main(int argc, char **argv) {
  int __attribute__((unused)) ret = chdir("prototype2");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
