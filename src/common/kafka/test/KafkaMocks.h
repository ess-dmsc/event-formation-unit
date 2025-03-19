// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Code for mocking the librdkafka library.
///
//===----------------------------------------------------------------------===//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <librdkafka/rdkafkacpp.h>
#pragma GCC diagnostic pop

#include <gmock/gmock.h>

// GCOVR_EXCL_START

class MockProducer : public RdKafka::Producer {
public:
  MOCK_METHOD(std::string, name, (), (const, override));
  MOCK_METHOD(std::string, memberid, (), (const, override));
  // New with librdkafka 2.0.2
  MOCK_METHOD(RdKafka::Error *, sasl_set_credentials,
              (const std::string &, const std::string &), (override));
  // end librdkafka 2.0.2
  // New with librdkafka 1.9.2
  MOCK_METHOD(RdKafka::Error *, sasl_background_callbacks_enable, (),
              (override));
  MOCK_METHOD(RdKafka::Queue *, get_sasl_queue, (), (override));
  MOCK_METHOD(RdKafka::Queue *, get_background_queue, (), (override));
  void *mem_malloc(size_t size) override { return malloc(size); }
  void mem_free(void *ptr) override { return free(ptr); }
  // end librdkafka 1.9.2
  MOCK_METHOD(int, poll, (int), (override));
  MOCK_METHOD(int, outq_len, (), (override));
  MOCK_METHOD(RdKafka::ErrorCode, metadata,
              (bool, const RdKafka::Topic *, RdKafka::Metadata **, int),
              (override));
  MOCK_METHOD(RdKafka::ErrorCode, pause,
              (std::vector<RdKafka::TopicPartition *> &), (override));
  MOCK_METHOD(RdKafka::ErrorCode, resume,
              (std::vector<RdKafka::TopicPartition *> &), (override));
  MOCK_METHOD(RdKafka::ErrorCode, query_watermark_offsets,
              (const std::string &, int32_t, int64_t *, int64_t *, int),
              (override));
  MOCK_METHOD(RdKafka::ErrorCode, get_watermark_offsets,
              (const std::string &, int32_t, int64_t *, int64_t *), (override));
  MOCK_METHOD(RdKafka::ErrorCode, offsetsForTimes,
              (std::vector<RdKafka::TopicPartition *> &, int), (override));
  MOCK_METHOD(RdKafka::Queue *, get_partition_queue,
              (const RdKafka::TopicPartition *), (override));
  MOCK_METHOD(RdKafka::ErrorCode, set_log_queue, (RdKafka::Queue *),
              (override));
  MOCK_METHOD(void, yield, (), (override));
  MOCK_METHOD(std::string, clusterid, (int), (override));
  MOCK_METHOD(rd_kafka_s *, c_ptr, (), (override));
  MOCK_METHOD(RdKafka::Producer *, create, (RdKafka::Conf *, std::string));
  MOCK_METHOD(RdKafka::ErrorCode, produce,
              (RdKafka::Topic *, int32_t, int, void *, size_t,
               const std::string *, void *),
              (override));
  MOCK_METHOD(RdKafka::ErrorCode, produce,
              (RdKafka::Topic *, int32_t, int, void *, size_t, const void *,
               size_t, void *),
              (override));
  MOCK_METHOD(RdKafka::ErrorCode, produce,
              (const std::string, int32_t, int, void *, size_t, const void *,
               size_t, int64_t, void *),
              (override));
  MOCK_METHOD(RdKafka::ErrorCode, produce,
              (RdKafka::Topic *, int32_t, const std::vector<char> *,
               const std::vector<char> *, void *),
              (override));
  MOCK_METHOD(RdKafka::ErrorCode, flush, (int), (override));
  MOCK_METHOD(int32_t, controllerid, (int), (override));
  MOCK_METHOD(RdKafka::ErrorCode, fatal_error, (std::string &),
              (const, override));
  MOCK_METHOD(RdKafka::ErrorCode, oauthbearer_set_token,
              (const std::string &, int64_t, const std::string &,
               const std::list<std::string> &, std::string &),
              (override));
  MOCK_METHOD(RdKafka::ErrorCode, oauthbearer_set_token_failure,
              (const std::string &), (override));
  MOCK_METHOD(RdKafka::ErrorCode, purge, (int), (override));
  MOCK_METHOD(RdKafka::Error *, init_transactions, (int), (override));
  MOCK_METHOD(RdKafka::Error *, begin_transaction, (), (override));
  MOCK_METHOD(RdKafka::Error *, send_offsets_to_transaction,
              (const std::vector<RdKafka::TopicPartition *> &,
               const RdKafka::ConsumerGroupMetadata *, int),
              (override));
  MOCK_METHOD(RdKafka::Error *, commit_transaction, (int), (override));
  MOCK_METHOD(RdKafka::Error *, abort_transaction, (int), (override));
  MOCK_METHOD(RdKafka::ErrorCode, produce,
              (std::string, int32_t, int, void *, size_t, const void *, size_t,
               int64_t, RdKafka::Headers *, void *),
              (override));
};

// GCOVR_EXCL_STOP
