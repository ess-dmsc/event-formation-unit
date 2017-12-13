//
// Created by Jonas Nilsson on 2017-11-08.
//

#include <gtest/gtest.h>
#include <fstream>
#include "../AdcParse.h"

class AdcParsing : public ::testing::Test {
public:
  static void SetUpTestCas() {
    
  }
  virtual void SetUp() {
    std::ifstream PacketFile("test_packet_1.dat", std::ios::binary);
    ASSERT_TRUE(PacketFile.good());
    PacketFile.read(reinterpret_cast<char*>(&Packet.Data), 1470);
    ASSERT_TRUE(PacketFile.good());
    Packet.Length = 1470;
  }
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
    DataPointer[0].Data.Length = htons(5);
    DataPointer[0].Trailer = htonl(0xBEEFCAFE);
    DataPointer[0].Data.TimeStampSecondsFrac = htonl(0x0000FFFF);
    DataPointer[0].Data.TimeStampSeconds = htonl(0xAAAA0000);
    DataPointer[0].Data.Channel = htons(0xAA00);
    DataPointer[0].TestData[0] = htons(0xFF00);
    DataPointer[0].TestData[1] = htons(0x00FF);
    
    DataPointer[1].Data.MagicValue = htons(0xABCD);
    DataPointer[1].Trailer = htonl(0xBEEFCAFE);
    DataPointer[1].Data.Length = htons(5);
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
  EXPECT_EQ(ChannelData.Modules[0].Data.size(), 2);
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


