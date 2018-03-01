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

class AdcParsingStream : public ::testing::Test {
public:
  virtual void SetUp() {
    std::string PacketPath = TEST_PACKET_PATH;
    std::ifstream PacketFile(PacketPath + "test_packet_stream.dat", std::ios::binary);
    ASSERT_TRUE(PacketFile.good());
    PacketFile.read(reinterpret_cast<char*>(&Packet.Data), 1470);
    ASSERT_TRUE(PacketFile.good());
    Packet.Length = 1470;
  }
  InData Packet;
};

class AdcParsingStreamFail : public ::testing::Test {
public:
  virtual void SetUp() {
    std::string PacketPath = TEST_PACKET_PATH;
    std::ifstream PacketFile(PacketPath + "test_packet_stream.dat", std::ios::binary);
    ASSERT_TRUE(PacketFile.good());
    PacketFile.read(reinterpret_cast<char*>(&Packet.Data), 1470);
    ASSERT_TRUE(PacketFile.good());
    Packet.Length = 1470;
    Header = reinterpret_cast<PacketHeader*>(Packet.Data);
    StreamHead = reinterpret_cast<StreamHeader*>(Packet.Data + sizeof(PacketHeader));
    BEEFCAFE = Packet.Data + 1470 - 8;
  }
  std::uint8_t *BEEFCAFE;
  PacketHeader *Header;
  StreamHeader *StreamHead;
  InData Packet;
};

TEST_F(AdcParsing, ParseCorrectHeader) {
  HeaderInfo Header;
  EXPECT_NO_THROW(Header = parseHeader(Packet));
  EXPECT_EQ(Header.Type, PacketType::Data);
  EXPECT_GT(Header.DataStart, 0);
}

TEST_F(AdcParsing, ParseCorrectDataModule) {
  HeaderInfo Header;
  EXPECT_NO_THROW(Header = parseHeader(Packet));
  if (Header.Type == PacketType::Data) {
    AdcData ChannelData = parseData(Packet, Header.DataStart);
    EXPECT_EQ(ChannelData.Modules.size(), 1u);
    EXPECT_NE(ChannelData.FillerStart, Header.DataStart);
    EXPECT_NE(ChannelData.FillerStart, 0);
  } else {
    FAIL();
  }
}

TEST_F(AdcParsing, ParseCorrectTrailer) {
  HeaderInfo Header;
  EXPECT_NO_THROW(Header = parseHeader(Packet));
  if (Header.Type == PacketType::Data) {
    AdcData ChannelData = parseData(Packet, Header.DataStart);
    TrailerInfo Trailer = parseTrailer(Packet, ChannelData.FillerStart);
    EXPECT_EQ(Trailer.FillerBytes, Packet.Length - ChannelData.FillerStart - 4);
  } else {
    FAIL();
  }
}

TEST_F(AdcParsing, ParseCorrectPacket) {
  PacketData ResultingData;
  EXPECT_NO_THROW(ResultingData = parsePacket(Packet));
  EXPECT_EQ(ResultingData.Modules.size(), 1u);
}

TEST_F(AdcParsingStream, ParseCorrectStreamPacket) {
  PacketData ResultingData;
  EXPECT_NO_THROW(ResultingData = parsePacket(Packet));
  ASSERT_EQ(ResultingData.Modules.size(), 4u);
  EXPECT_EQ(ResultingData.Modules.at(0).OversamplingFactor, 4);
  std::uint32_t ExpectedTimeStampSeconds = 101;
  std::uint32_t ExpectedTimeStampSecondsFrac = 22974588;
  for (auto &Module : ResultingData.Modules) {
    EXPECT_EQ(Module.TimeStampSeconds, ExpectedTimeStampSeconds);
    EXPECT_EQ(Module.TimeStampSecondsFrac, ExpectedTimeStampSecondsFrac);
  }
}

TEST_F(AdcParsingStreamFail, LengthFail) {
  StreamHead->fixEndian();
  StreamHead->Length = 500;
  StreamHead->fixEndian();
  EXPECT_THROW(parsePacket(Packet), ParserException);
}

TEST_F(AdcParsingStreamFail, ABCDFail) {
  StreamHead->fixEndian();
  StreamHead->MagicValue = 0xCDAB;
  StreamHead->fixEndian();
  EXPECT_THROW(parsePacket(Packet), ParserException);
}

TEST_F(AdcParsingStreamFail, HeaderLengthFail) {
  Packet.Length = sizeof(PacketHeader) + 10;
  Header->fixEndian();
  Header->ReadoutLength = sizeof(PacketHeader) + 8;
  Header->fixEndian();
  EXPECT_THROW(parsePacket(Packet), ParserException);
}

TEST_F(AdcParsingStreamFail, BEEFCAFEFail) {
  *BEEFCAFE = 0X00;
  EXPECT_THROW(parsePacket(Packet), ParserException);
}

TEST_F(AdcParsingIdle, ParseCorrectIdlePacket) {
  PacketData ResultingData;
  EXPECT_NO_THROW(ResultingData = parsePacket(Packet));
  EXPECT_EQ(ResultingData.Modules.size(), 0u);
  EXPECT_EQ(ResultingData.IdleTimeStampSeconds, 0xAAAA0000);
  EXPECT_EQ(ResultingData.IdleTimeStampSecondsFrac, 0x0000AAAA);
}

TEST(AdcStreamSetting, IncorrectFirstByte) {
  std::uint16_t Setting = 0x31f4;
  EXPECT_THROW(parseStreamSettings(Setting), ParserException);
}

TEST(AdcStreamSetting, IncorrectOversampling1) {
  std::uint16_t Setting = 0x33f5;
  EXPECT_THROW(parseStreamSettings(Setting), ParserException);
}

TEST(AdcStreamSetting, IncorrectOversampling2) {
  std::uint16_t Setting = 0x3331;
  EXPECT_THROW(parseStreamSettings(Setting), ParserException);
}

TEST(AdcStreamSetting, IncorrectOversampling3) {
  std::uint16_t Setting = 0x33f0;
  EXPECT_THROW(parseStreamSettings(Setting), ParserException);
}

TEST(AdcStreamSetting, CorrectSetting) {
  std::uint16_t Setting = 0x33f4;
  EXPECT_NO_THROW(parseStreamSettings(Setting));
  auto CurrentSetting = parseStreamSettings(Setting);
  EXPECT_EQ(CurrentSetting.OversamplingFactor, 4);
  EXPECT_EQ(CurrentSetting.ChannelsActive.size(), 4);
  std::vector<int> ExpectedChannels{0, 1, 2, 3};
  EXPECT_EQ(CurrentSetting.ChannelsActive, ExpectedChannels);
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
  EXPECT_EQ(IdleResult.TimeStampSeconds, TimeStampValue);
  EXPECT_EQ(IdleResult.TimeStampSecondsFrac, TimeStampValueFrac);
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
  EXPECT_NE(IdleResult.TimeStampSeconds, TimeStampValue);
  EXPECT_NE(IdleResult.TimeStampSecondsFrac, TimeStampValueFrac);
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
  std::string ExpectedExceptionString("ParserException error string not defined for exception of type " + std::to_string(ExceptionTypeInt));
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
    DataPointer[0].Data.TimeStampSecondsFrac = htonl(0x0000FFFF);
    DataPointer[0].Data.TimeStampSeconds = htonl(0xAAAA0000);
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
  AdcData ChannelData;
  EXPECT_NO_THROW(ChannelData = parseData(Packet, 0));
  EXPECT_EQ(ChannelData.Modules.size(), 2u);
  EXPECT_EQ(ChannelData.Modules[0].TimeStampSecondsFrac, 0x0000FFFFu);
  EXPECT_EQ(ChannelData.Modules[0].TimeStampSeconds, 0xAAAA0000u);
  EXPECT_EQ(ChannelData.Modules[0].Channel, 0xAA00);
  EXPECT_EQ(ChannelData.Modules[0].Data.size(), 2u);
  EXPECT_EQ(ChannelData.Modules[0].Data[0], 0xFF00);
  EXPECT_EQ(ChannelData.Modules[0].Data[1], 0x00FF);
}

TEST_F(AdcDataParsing, MagicWordFail) {
  DataPointer[1].Data.MagicValue = 0x0000;
  EXPECT_THROW(parseData(Packet, 0), ParserException);
}

TEST_F(AdcDataParsing, TrailerFail) {
  DataPointer[1].Trailer = 0;
  EXPECT_THROW(parseData(Packet, 0), ParserException);
}

TEST_F(AdcDataParsing, NrOfSamplesFail) {
  DataPointer[1].Data.Length = 20;
  EXPECT_THROW(parseData(Packet, 0), ParserException);
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

