/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Unit tests.
 */

#include <gtest/gtest.h>
#include <fstream>
#include "../AdcParse.h"

class AdcParsing : public ::testing::Test {
public:
  virtual void SetUp() {
    std::string PacketPath = TEST_PACKET_PATH;
    std::ifstream PacketFile(PacketPath + "test_packet_1.dat", std::ios::binary);
    ASSERT_TRUE(PacketFile.good());
    PacketFile.read(reinterpret_cast<char*>(&Packet.Data), 1470);
    ASSERT_TRUE(PacketFile.good());
    Packet.Length = 1470;
  }
  InData Packet;
};

class AdcParsingAlt : public ::testing::Test {
public:
  virtual void SetUp() {
    std::string PacketPath = TEST_PACKET_PATH;
    std::ifstream PacketFile(PacketPath + "test_packet_2.dat", std::ios::binary);
    ASSERT_TRUE(PacketFile.good());
    PacketFile.read(reinterpret_cast<char*>(&Packet.Data), 9186);
    ASSERT_TRUE(PacketFile.good());
    Packet.Length = 9186;
  }
  InData Packet;
};

class AdcParsingIdle : public ::testing::Test {
public:
  virtual void SetUp() {
    std::string PacketPath = TEST_PACKET_PATH;
    std::ifstream PacketFile(PacketPath + "test_packet_idle.dat", std::ios::binary);
    ASSERT_TRUE(PacketFile.good());
    PacketFile.read(reinterpret_cast<char*>(&Packet.Data), 22);
    ASSERT_TRUE(PacketFile.good());
    Packet.Length = 22;
  }
  InData Packet;
};

class AdcParsingDataFail : public ::testing::Test {
public:
  virtual void SetUp() {
    std::string PacketPath = TEST_PACKET_PATH;
    std::ifstream PacketFile(PacketPath + "test_packet_2.dat", std::ios::binary);
    ASSERT_TRUE(PacketFile.good());
    PacketFile.read(reinterpret_cast<char*>(&Packet.Data), 1470);
    ASSERT_TRUE(PacketFile.good());
    Packet.Length = 1470;
    Header = reinterpret_cast<PacketHeader*>(Packet.Data);
    DataHead = reinterpret_cast<DataHeader*>(Packet.Data + sizeof(PacketHeader));
    BEEFCAFE = Packet.Data + 1470 - 8;
  }
  std::uint8_t *BEEFCAFE;
  PacketHeader *Header;
  DataHeader *DataHead;
  InData Packet;
};

TEST_F(AdcParsing, ParseCorrectHeader) {
  HeaderInfo Header;
  EXPECT_NO_THROW(Header = parseHeader(Packet));
  EXPECT_EQ(Header.Type, PacketType::Data);
  EXPECT_GT(Header.DataStart, 0);
}

DataModule* GetModule(int) {
  static DataModule TestModule(10);
  return &TestModule;
}

class ParserStandIn : public PacketParser {
public:
  ParserStandIn(std::function<bool(DataModule*)> ModuleHandler, std::function<DataModule*(int Channel)> ModuleProducer) : PacketParser(ModuleHandler, ModuleProducer) {
  }
  using PacketParser::parseData;
};

TEST_F(AdcParsing, ParseCorrectDataModule) {
  int NrOfModules{0};
  std::function<bool(DataModule*)> ProccessingFunction([&NrOfModules](DataModule*){
    NrOfModules++;
    return true;
  });
  ParserStandIn Parser(ProccessingFunction, GetModule);
  HeaderInfo Header;
  EXPECT_NO_THROW(Header = parseHeader(Packet));
  if (Header.Type == PacketType::Data) {
    size_t FillerStart = Parser.parseData(Packet, Header.DataStart);
    EXPECT_EQ(NrOfModules, 1);
    EXPECT_NE(FillerStart, static_cast<size_t>(Header.DataStart));
    EXPECT_NE(FillerStart, 0u);
  } else {
    FAIL();
  }
}

TEST_F(AdcParsing, ParseCorrectTrailer) {
  std::function<bool(DataModule*)> ProccessingFunction([](DataModule*){
    return true;
  });
  ParserStandIn Parser(ProccessingFunction, GetModule);
  HeaderInfo Header;
  EXPECT_NO_THROW(Header = parseHeader(Packet));
  if (Header.Type == PacketType::Data) {
    size_t FillerStart = Parser.parseData(Packet, Header.DataStart);
    TrailerInfo Trailer = parseTrailer(Packet, FillerStart);
    EXPECT_EQ(Trailer.FillerBytes, Packet.Length - FillerStart - 4);
  } else {
    FAIL();
  }
}

TEST_F(AdcParsingAlt, ParseCorrectPacket) {
  int NrOfModules{0};
  int OversamplingFactor;
  std::function<bool(DataModule*)> ProccessingFunction([&NrOfModules,&OversamplingFactor](DataModule* Module){
    OversamplingFactor = Module->OversamplingFactor;
    NrOfModules++;
    return true;
  });
  ParserStandIn Parser(ProccessingFunction, GetModule);
  PacketInfo ParseResult;
  EXPECT_NO_THROW(ParseResult = Parser.parsePacket(Packet));
  EXPECT_EQ(NrOfModules, 1);
  EXPECT_EQ(OversamplingFactor, 4);
}

TEST_F(AdcParsing, ParseCorrectPacket) {
  int NrOfModules{0};
  std::function<bool(DataModule*)> ProccessingFunction([&NrOfModules](DataModule*){
    NrOfModules++;
    return true;
  });
  ParserStandIn Parser(ProccessingFunction, GetModule);
  PacketInfo ResultingData;
  EXPECT_NO_THROW(ResultingData = Parser.parsePacket(Packet));
  EXPECT_EQ(NrOfModules, 1);
}

TEST_F(AdcParsingDataFail, LengthFail) {
  std::function<bool(DataModule*)> ProccessingFunction([](DataModule*){
    return true;
  });
  ParserStandIn Parser(ProccessingFunction, GetModule);
  DataHead->fixEndian();
  DataHead->Length = 500;
  DataHead->fixEndian();
  EXPECT_THROW(Parser.parsePacket(Packet), ParserException);
}

TEST_F(AdcParsingDataFail, ABCDFail) {
  std::function<bool(DataModule*)> ProccessingFunction([](DataModule*){
    return true;
  });
  ParserStandIn Parser(ProccessingFunction, GetModule);
  DataHead->fixEndian();
  DataHead->MagicValue = 0xCDAB;
  DataHead->fixEndian();
  EXPECT_THROW(Parser.parsePacket(Packet), ParserException);
}

TEST_F(AdcParsingDataFail, HeaderLengthFail) {
  std::function<bool(DataModule*)> ProccessingFunction([](DataModule*){
    return true;
  });
  ParserStandIn Parser(ProccessingFunction, GetModule);
  Packet.Length = sizeof(PacketHeader) + 10;
  Header->fixEndian();
  Header->ReadoutLength = sizeof(PacketHeader) + 8;
  Header->fixEndian();
  EXPECT_THROW(Parser.parsePacket(Packet), ParserException);
}

TEST_F(AdcParsingDataFail, BEEFCAFEFail) {
  std::function<bool(DataModule*)> ProccessingFunction([](DataModule*){
    return true;
  });
  ParserStandIn Parser(ProccessingFunction, GetModule);
  *BEEFCAFE = 0X00;
  EXPECT_THROW(Parser.parsePacket(Packet), ParserException);
}

TEST_F(AdcParsingIdle, ParseCorrectIdlePacket) {
  int NrOfModules{0};
  std::function<bool(DataModule*)> ProccessingFunction([&NrOfModules](DataModule*){
    NrOfModules++;
    return true;
  });
  ParserStandIn Parser(ProccessingFunction, GetModule);
  PacketInfo ResultingData;
  EXPECT_NO_THROW(ResultingData = Parser.parsePacket(Packet));
  EXPECT_EQ(NrOfModules, 0);
  //EXPECT_EQ(ResultingData.IdleTimeStamp.Seconds, 0xAAAA0000u);
  //EXPECT_EQ(ResultingData.IdleTimeStamp.SecondsFrac, 0x0000AAAAu);
}

TEST(AdcHeadParse, IdleHeadTest) {
  InData Packet;
  Packet.Length = sizeof(PacketHeader);
  PacketHeader *HeaderPointer = reinterpret_cast<PacketHeader*>(Packet.Data);
  HeaderPointer->PacketType = 0x2222;
  HeaderPointer->ReadoutLength = htons(sizeof(PacketHeader) - 2);
  HeaderInfo Header;
  EXPECT_NO_THROW(Header = parseHeader(Packet));
  EXPECT_EQ(Header.Type, PacketType::Idle);
}

TEST(AdcHeadParse, IdleDataTest) {
  InData Packet;
  std::uint32_t *IdleData = reinterpret_cast<std::uint32_t*>(Packet.Data);
  const std::uint32_t TimeStampValue = 0xFF;
  const std::uint32_t TimeStampValueFrac = 0xFFFF0000;
  IdleData[0] = htonl(TimeStampValue);
  IdleData[1] = htonl(TimeStampValueFrac);
  Packet.Length = sizeof(std::uint32_t) * 2;
  IdleInfo IdleResult;
  EXPECT_NO_THROW(IdleResult = parseIdle(Packet, 0));
  EXPECT_EQ(IdleResult.TimeStamp.Seconds, TimeStampValue);
  EXPECT_EQ(IdleResult.TimeStamp.SecondsFrac, TimeStampValueFrac);
}

TEST(AdcHeadParse, IdleDataFailTest) {
  InData Packet;
  std::uint32_t *IdleData = reinterpret_cast<std::uint32_t*>(Packet.Data);
  const std::uint32_t TimeStampValue = 0xFF;
  const std::uint32_t TimeStampValueFrac = 0xFFFF0000;
  IdleData[0] = TimeStampValue;
  IdleData[1] = TimeStampValueFrac;
  Packet.Length = sizeof(std::uint32_t) * 2;
  IdleInfo IdleResult;
  EXPECT_NO_THROW(IdleResult = parseIdle(Packet, 0));
  EXPECT_NE(IdleResult.TimeStamp.Seconds, TimeStampValue);
  EXPECT_NE(IdleResult.TimeStamp.SecondsFrac, TimeStampValueFrac);
}

TEST(AdcHeadParse, IdleDataFail) {
  InData Packet;
  Packet.Length = 2;
  EXPECT_THROW(parseIdle(Packet, 0), ParserException);
}

TEST(ExceptionTypes, ExceptionTypeTest) {
  ParserException::Type SomeType(ParserException::Type::TRAILER_FEEDF00D);
  ParserException SomeException(SomeType);
  EXPECT_EQ(SomeException.getErrorType(), SomeType);
}

TEST(ExceptionTypes, InitWithString) {
  std::string SomeTestString = "Something";
  ParserException SomeException(SomeTestString);
  EXPECT_STREQ(SomeException.what(), SomeTestString.c_str());
  EXPECT_EQ(SomeException.getErrorType(), ParserException::Type::UNKNOWN);
}

TEST(ExceptionTypes, IncorrectExceptionType) {
  int ExceptionTypeInt = 4242;
  ParserException SomeException((ParserException::Type(ExceptionTypeInt)));
  std::string ExpectedExceptionString("ParserException error string not defined for exceptions of this type.");
  EXPECT_STREQ(SomeException.what(), ExpectedExceptionString.c_str());
}

TEST(AdcHeadParse, UnknownHeadTest) {
  InData Packet;
  Packet.Length = sizeof(PacketHeader);
  PacketHeader *HeaderPointer = reinterpret_cast<PacketHeader*>(Packet.Data);
  HeaderPointer->PacketType = 0x6666;
  HeaderPointer->ReadoutLength = htons(sizeof(PacketHeader) - 2);
  EXPECT_THROW(parseHeader(Packet), ParserException);
}

TEST(AdcHeadParse, ShortPacketFailure) {
  InData Packet;
  Packet.Length = sizeof(PacketHeader) - 2;
  PacketHeader *HeaderPointer = reinterpret_cast<PacketHeader*>(Packet.Data);
  HeaderPointer->PacketType = 0x1111;
  HeaderPointer->ReadoutLength = htons(sizeof(PacketHeader) - 4);
  EXPECT_THROW(parseHeader(Packet), ParserException);
}

TEST(AdcHeadParse, WrongReadoutLengthFailure) {
  InData Packet;
  Packet.Length = sizeof(PacketHeader);
  PacketHeader *HeaderPointer = reinterpret_cast<PacketHeader*>(Packet.Data);
  HeaderPointer->PacketType = 0x1111;
  HeaderPointer->ReadoutLength = htons(sizeof(PacketHeader));
  EXPECT_THROW(parseHeader(Packet), ParserException);
}

struct FakeDataStruct {
  DataHeader Data;
  std::uint16_t TestData[2];
  std::uint32_t Trailer;
};

class AdcDataParsing : public ::testing::Test {
public:
  virtual void SetUp() {
    Packet.Length = sizeof(FakeDataStruct) * 2;
    DataPointer = reinterpret_cast<FakeDataStruct*>(Packet.Data);
    DataPointer[0].Data.MagicValue = htons(0xABCD);
    DataPointer[0].Data.Length = htons(24);
    DataPointer[0].Trailer = htonl(0xBEEFCAFE);
    DataPointer[0].Data.TimeStamp.SecondsFrac = htonl(0x0000FFFF);
    DataPointer[0].Data.TimeStamp.Seconds = htonl(0xAAAA0000);
    DataPointer[0].Data.Channel = htons(0xAA00);
    DataPointer[0].TestData[0] = htons(0xFF00);
    DataPointer[0].TestData[1] = htons(0x00FF);
    
    DataPointer[1].Data.MagicValue = htons(0xABCD);
    DataPointer[1].Trailer = htonl(0xBEEFCAFE);
    DataPointer[1].Data.Length = htons(24);
  }
  InData Packet;
  FakeDataStruct *DataPointer;
};

TEST_F(AdcDataParsing, FakeDataTest) {
  int NrOfModules{0};
  DataModule ModulePtr;
  bool FirstModule = true;
  std::function<bool(DataModule*)> ProccessingFunction([&NrOfModules,&ModulePtr,&FirstModule](DataModule *NewModule){
    NrOfModules++;
    if (FirstModule) {
      FirstModule = false;
      ModulePtr = *NewModule;
    }
    return true;
  });
  ParserStandIn Parser(ProccessingFunction, GetModule);
  
  EXPECT_NO_THROW(Parser.parseData(Packet, 0));
  EXPECT_EQ(NrOfModules, 2);
  EXPECT_EQ(ModulePtr.TimeStamp.SecondsFrac, 0x0000FFFFu);
  EXPECT_EQ(ModulePtr.TimeStamp.Seconds, 0xAAAA0000u);
  EXPECT_EQ(ModulePtr.Channel, 0xAA00);
  EXPECT_EQ(ModulePtr.Data.size(), 2u);
  EXPECT_EQ(ModulePtr.Data[0], 0xFF00);
  EXPECT_EQ(ModulePtr.Data[1], 0x00FF);
}

TEST_F(AdcDataParsing, MagicWordFail) {
  std::function<bool(DataModule*)> ProccessingFunction([](DataModule*){
    return true;
  });
  ParserStandIn Parser(ProccessingFunction, GetModule);
  DataPointer[1].Data.MagicValue = 0x0000;
  EXPECT_THROW(Parser.parseData(Packet, 0), ParserException);
}

TEST_F(AdcDataParsing, TrailerFail) {
  std::function<bool(DataModule*)> ProccessingFunction([](DataModule*){
    return true;
  });
  ParserStandIn Parser(ProccessingFunction, GetModule);
  DataPointer[1].Trailer = 0;
  EXPECT_THROW(Parser.parseData(Packet, 0), ParserException);
}

TEST_F(AdcDataParsing, NrOfSamplesFail) {
  std::function<bool(DataModule*)> ProccessingFunction([](DataModule*){
    return true;
  });
  ParserStandIn Parser(ProccessingFunction, GetModule);
  DataPointer[1].Data.Length = 20;
  EXPECT_THROW(Parser.parseData(Packet, 0), ParserException);
}

struct FillerDataStruct1 {
  std::uint8_t FillerBytes[4];
  std::uint32_t Trailer;
};

class AdcFillerParsing1 : public ::testing::Test {
public:
  virtual void SetUp() {
    Packet.Length = sizeof(FillerDataStruct1);
    FillerPointer = reinterpret_cast<FillerDataStruct1*>(Packet.Data);
    for (auto &Fill : FillerPointer->FillerBytes) {
      Fill = 0x55;
    }
    FillerPointer->Trailer = htonl(0xFEEDF00D);
  }
  InData Packet;
  FillerDataStruct1 *FillerPointer;
};

TEST_F(AdcFillerParsing1, FakeFillerTest) {
  TrailerInfo Trailer;
  EXPECT_NO_THROW(Trailer = parseTrailer(Packet, 0));
  EXPECT_EQ(Trailer.FillerBytes, 4u);
}

TEST_F(AdcFillerParsing1, IncorrectTrailer) {
  FillerPointer->Trailer = 0x0000;
  EXPECT_THROW(parseTrailer(Packet, 0), ParserException);
  
}

TEST_F(AdcFillerParsing1, IncorrectFiller) {
  FillerPointer->FillerBytes[0] = 0x11;
  EXPECT_THROW(parseTrailer(Packet, 0), ParserException);
}

class AdcFillerParsing2 : public ::testing::Test {
public:
  virtual void SetUp() {
    Packet.Length = sizeof(FillerDataStruct1) - 1;
    FillerPointer = reinterpret_cast<FillerDataStruct1*>(Packet.Data);
    for (auto &Fill : FillerPointer->FillerBytes) {
      Fill = 0x55;
    }
    FillerPointer->Trailer = htonl(0xFEEDF00D);
  }
  InData Packet;
  FillerDataStruct1 *FillerPointer;
};

TEST_F(AdcFillerParsing2, IncorrectFillerLength) {
  EXPECT_THROW(parseTrailer(Packet, 0), ParserException);
}

