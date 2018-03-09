/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief For doing time stamp calculations.
 */

#pragma once

#include <cstdint>
#include <netinet/in.h>

struct RawTimeStamp {
  RawTimeStamp() = default;
  RawTimeStamp(std::uint32_t Sec, std::uint32_t SecFrac) : Seconds(Sec), SecondsFrac(SecFrac) {}
  std::uint32_t Seconds{0};
  std::uint32_t SecondsFrac{0};
  void fixEndian() {
    Seconds = ntohl(Seconds);
    SecondsFrac = ntohl(SecondsFrac);
  }
  RawTimeStamp GetOffsetTimeStamp(const std::int32_t &SampleOffset) const;
    
  std::uint64_t GetTimeStampNS() const;
    
  //Note: This function might be significantly (a lot) slower than TimeStamp::Calc() for some cases. The reverse is also true.
  std::uint64_t GetTimeStampNSFast() const;
} __attribute__((packed));
