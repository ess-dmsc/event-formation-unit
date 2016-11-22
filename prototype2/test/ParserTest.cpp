/** Copyright (C) 2016 European Spallation Source ERIC */

#include "TestBase.h"
#include <algorithm>
#include <common/EFUArgs.h>
#include <cstring>
#include <efu/Parser.h>
#include <memory>

using namespace std;

// clang-format off
std::vector<std::string> commands {
  "STAT_INPUT",                     "STAT_INPUT 0, 0, 0, 0",
  "STAT_PROCESSING",                "STAT_PROCESSING 0, 0, 0, 0, 0, 0",
  "STAT_OUTPUT",                    "STAT_OUTPUT 0, 0, 0",
  "STAT_MASK_SET 0",                "<OK>",
  "STAT_RESET",                     "<OK>",
  // doesnt work when tests are called outside prototype2/ dir
  //"CSPEC_LOAD_CALIB data/cal_zero", "<OK>",
  "CSPEC_SHOW_CALIB",               "wire 0 0x0000, grid 0 0x0000",
  "CSPEC_SHOW_CALIB 5",             "wire 5 0x0000, grid 5 0x0000",
};
// clang-format on

class ParserTest : public TestBase {
protected:
  Parser *parser;

  virtual void SetUp() {
    parser = new Parser();
    efu_args = new EFUArgs(0, NULL);
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
  ASSERT_EQ( strcmp("Error: <SIZE>", output) , 0);

  input[0] = 'A';
  input[1] = 'B';
  res = parser->parse(input, 1, output, &obytes);
  ASSERT_EQ('A', input[0]);
  ASSERT_EQ('\0', input[1]);
  ASSERT_EQ( strcmp("Error: <BADCMD>", output) , 0);
  ASSERT_EQ(-Parser::EBADCMD, res);

  res = parser->parse(input, 2, output, &obytes);
  ASSERT_EQ('A', input[0]);
  ASSERT_EQ('\0', input[1]);
  ASSERT_EQ( strcmp("Error: <BADCMD>", output) , 0);
  ASSERT_EQ(-Parser::EBADCMD, res);

  input[0] = 'A';
  input[1] = '\n';
  input[2] = '\0';
  res = parser->parse(input, 3, output, &obytes);
  ASSERT_EQ('A', input[0]);
  ASSERT_EQ('\0', input[1]);
  ASSERT_EQ('\0', input[2]);
  ASSERT_EQ( strcmp("Error: <BADCMD>", output) , 0);
  ASSERT_EQ(-Parser::EBADCMD, res);
}

TEST_F(ParserTest, OversizeData) {
  memset(input, 0x41, buffer_size);
  MESSAGE() << "Max buffer size\n";
  auto res = parser->parse(input, buffer_size, output, &obytes);
  ASSERT_EQ('\0', input[buffer_size - 1]);
  ASSERT_EQ( strcmp("Error: <BADCMD>", output) , 0);
  ASSERT_EQ(-Parser::EBADCMD, res);

  MESSAGE() << "Max buffer size + 1\n";
  res = parser->parse(input, buffer_size + 1, output, &obytes);
  ASSERT_EQ(-Parser::EOSIZE, res);
  ASSERT_EQ( strcmp("Error: <SIZE>", output) , 0);
}

TEST_F(ParserTest, NoTokens) {
  memset(input, 0x20, buffer_size);
  MESSAGE() << "Spaces only\n";
  auto res = parser->parse(input, buffer_size, output, &obytes);
  ASSERT_EQ(-Parser::ENOTOKENS, res);
  ASSERT_EQ( strcmp("Error: <BADCMD>", output) , 0);
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

TEST_F(ParserTest, InvalidCommandStatMask) {
  char *cmd = (char *)"STAT_MASK_SET";
  std::memcpy(input, cmd, strlen(cmd));

  auto res = parser->parse(input, strlen(cmd), output, &obytes);
  ASSERT_EQ( strcmp("Error: <BADARGS>", output) , 0);
  ASSERT_EQ(-Parser::EBADARGS, res);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
