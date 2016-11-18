/** Copyright (C) 2016 European Spallation Source ERIC */

#include "TestBase.h"
#include <algorithm>
#include <cstring>
#include <common/EFUArgs.h>
#include <efu/Parser.h>
#include <memory>

using namespace std;


class ParserTest : public TestBase {
protected:
  static const unsigned int buffer_size = 9000;
  char * inputbuffer[buffer_size];
  unsigned int ibytes, obytes;
  char * outputbuffer[buffer_size];

  char * input = (char*)inputbuffer;
  char * output = (char*)outputbuffer;
};

/** Test cases below */
TEST_F(ParserTest, InputBuffer) {
  EFUArgs args(0, NULL);
  Parser parser(args);

  auto res = parser.parse(input, 0, output, &obytes);
  ASSERT_EQ(-Parser::EUSIZE, res);
  ASSERT_EQ(0, obytes);

  input[0] = 'A';
  input[1] = 'B';
  res = parser.parse(input, 1 ,output, &obytes);
  ASSERT_EQ(-Parser::EUSIZE, res);
  ASSERT_EQ('A', input[0]);
  ASSERT_EQ('B', input[1]);
  ASSERT_EQ(0, obytes);

  res = parser.parse(input, 2 ,output, &obytes);
  ASSERT_EQ('A', input[0]);
  ASSERT_EQ('\0', input[1]);
  ASSERT_EQ(0, obytes);
  ASSERT_EQ(-Parser::EBADCMD, res);

  input[0] = 'A';
  input[1] = '\n';
  input[2] = '\0';
  res = parser.parse(input, 3 ,output, &obytes);
  ASSERT_EQ('A', input[0]);
  ASSERT_EQ('\0', input[1]);
  ASSERT_EQ('\0', input[2]);
  ASSERT_EQ(0, obytes);
  ASSERT_EQ(-Parser::EBADCMD, res);
}

TEST_F(ParserTest, OversizeData) {
  EFUArgs args(0, NULL);
  Parser parser(args);

  memset(input, 0x41, buffer_size);
  MESSAGE() << "Max buffer size\n";
  auto res = parser.parse(input, buffer_size, output, &obytes);
  ASSERT_EQ(-Parser::EBADCMD, res);
  ASSERT_EQ(0, obytes);

  MESSAGE() << "Max buffer size + 1\n";
  res = parser.parse(input, buffer_size + 1, output, &obytes);
  ASSERT_EQ(-Parser::EOSIZE, res);
  ASSERT_EQ(0, obytes);
}

TEST_F(ParserTest, NoTokens) {
  EFUArgs args(0, NULL);
  Parser parser(args);

  memset(input, 0x20, buffer_size);
  MESSAGE() << "Spaces only\n";
  auto res = parser.parse(input, buffer_size, output, &obytes);
  ASSERT_EQ(-Parser::ENOTOKENS, res);
  ASSERT_EQ(0, obytes);
}


TEST_F(ParserTest, ValidCommand) {
  EFUArgs args(0, NULL);
  Parser parser(args);

  char * stat_input = (char*)"STAT_INPUT ";
  std::memcpy(input, stat_input, strlen(stat_input));

  auto res = parser.parse(input, 12, output, &obytes);
  ASSERT_GT(obytes, strlen(stat_input));
  ASSERT_EQ(0, res);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
