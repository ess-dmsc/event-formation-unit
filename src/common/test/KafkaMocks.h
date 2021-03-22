/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
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

#include <trompeloeil.hpp>

// GCOVR_EXCL_START

class MockProducer : public trompeloeil::mock_interface<RdKafka::Producer> {
public:
  MAKE_CONST_MOCK0(name, const std::string(), override);
  MAKE_CONST_MOCK0(memberid, const std::string(), override);
  MAKE_MOCK1(poll, int(int), override);
  MAKE_MOCK0(outq_len, int(), override);
  MAKE_MOCK4(metadata,
             RdKafka::ErrorCode(bool, const RdKafka::Topic *,
                                RdKafka::Metadata **, int),
             override);
  MAKE_MOCK1(pause,
             RdKafka::ErrorCode(std::vector<RdKafka::TopicPartition *> &),
             override);
  MAKE_MOCK1(resume,
             RdKafka::ErrorCode(std::vector<RdKafka::TopicPartition *> &),
             override);
  MAKE_MOCK5(query_watermark_offsets,
             RdKafka::ErrorCode(const std::string &, int32_t, int64_t *,
                                int64_t *, int),
             override);
  MAKE_MOCK4(get_watermark_offsets,
             RdKafka::ErrorCode(const std::string &, int32_t, int64_t *,
                                int64_t *),
             override);
  MAKE_MOCK2(offsetsForTimes,
             RdKafka::ErrorCode(std::vector<RdKafka::TopicPartition *> &, int),
             override);
  MAKE_MOCK1(get_partition_queue,
             RdKafka::Queue *(const RdKafka::TopicPartition *), override);
  MAKE_MOCK1(set_log_queue, RdKafka::ErrorCode(RdKafka::Queue *), override);
  MAKE_MOCK0(yield, void(), override);
  MAKE_MOCK1(clusterid, const std::string(int), override);
  MAKE_MOCK0(c_ptr, rd_kafka_s *(), override);
  MAKE_MOCK2(create, RdKafka::Producer *(RdKafka::Conf *, std::string));
  MAKE_MOCK7(produce,
             RdKafka::ErrorCode(RdKafka::Topic *, int32_t, int, void *, size_t,
                                const std::string *, void *),
             override);
  MAKE_MOCK8(produce,
             RdKafka::ErrorCode(RdKafka::Topic *, int32_t, int, void *, size_t,
                                const void *, size_t, void *),
             override);
  MAKE_MOCK9(produce,
             RdKafka::ErrorCode(const std::string, int32_t, int, void *, size_t,
                                const void *, size_t, int64_t, void *),
             override);
  MAKE_MOCK5(produce,
             RdKafka::ErrorCode(RdKafka::Topic *, int32_t,
                                const std::vector<char> *,
                                const std::vector<char> *, void *),
             override);
  MAKE_MOCK1(flush, RdKafka::ErrorCode(int), override);
  MAKE_MOCK1(controllerid, int32_t(int), override);
  MAKE_CONST_MOCK1(fatal_error, RdKafka::ErrorCode(std::string &), override);
  MAKE_MOCK5(oauthbearer_set_token,
             RdKafka::ErrorCode(const std::string &, int64_t,
                                const std::string &,
                                const std::list<std::string> &, std::string &),
             override);
  MAKE_MOCK1(oauthbearer_set_token_failure,
             RdKafka::ErrorCode(const std::string &), override);
  MAKE_MOCK1(purge, RdKafka::ErrorCode(int), override);
  IMPLEMENT_MOCK1(init_transactions);
  IMPLEMENT_MOCK0(begin_transaction);
  IMPLEMENT_MOCK3(send_offsets_to_transaction);
  IMPLEMENT_MOCK1(commit_transaction);
  IMPLEMENT_MOCK1(abort_transaction);
  MAKE_MOCK10(produce,
              RdKafka::ErrorCode(std::string, int32_t, int, void *, size_t,
                                 const void *, size_t, int64_t,
                                 RdKafka::Headers *, void *),
              override);
};

class FakeTopic : public RdKafka::Topic {
public:
  FakeTopic() = default;
  ~FakeTopic() override = default;
  const std::string name() const override { return ""; };
  bool partition_available(int32_t) const override { return true; };
  RdKafka::ErrorCode offset_store(int32_t, int64_t) override {
    return RdKafka::ERR_NO_ERROR;
  };
  struct rd_kafka_topic_s *c_ptr() override {
    return {};
  };
};

class MockConf : public RdKafka::Conf {
public:
  MAKE_MOCK3(set,
             RdKafka::Conf::ConfResult(const std::string &, const std::string &,
                                       std::string &),
             override);
  MAKE_MOCK3(set,
             RdKafka::Conf::ConfResult(const std::string &,
                                       RdKafka::DeliveryReportCb *,
                                       std::string &),
             override);
  MAKE_MOCK3(set,
             RdKafka::Conf::ConfResult(const std::string &, RdKafka::EventCb *,
                                       std::string &),
             override);
  MAKE_MOCK3(set,
             RdKafka::Conf::ConfResult(const std::string &,
                                       const RdKafka::Conf *, std::string &),
             override);
  MAKE_MOCK3(set,
             RdKafka::Conf::ConfResult(const std::string &,
                                       RdKafka::PartitionerCb *, std::string &),
             override);
  MAKE_MOCK3(set,
             RdKafka::Conf::ConfResult(const std::string &,
                                       RdKafka::PartitionerKeyPointerCb *,
                                       std::string &),
             override);
  MAKE_MOCK3(set,
             RdKafka::Conf::ConfResult(const std::string &, RdKafka::SocketCb *,
                                       std::string &),
             override);
  MAKE_MOCK3(set,
             RdKafka::Conf::ConfResult(const std::string &, RdKafka::OpenCb *,
                                       std::string &),
             override);
  MAKE_MOCK3(set,
             RdKafka::Conf::ConfResult(const std::string &,
                                       RdKafka::RebalanceCb *, std::string &),
             override);
  MAKE_MOCK3(set,
             RdKafka::Conf::ConfResult(const std::string &,
                                       RdKafka::OffsetCommitCb *,
                                       std::string &),
             override);

  MAKE_CONST_MOCK2(get,
                   RdKafka::Conf::ConfResult(const std::string &,
                                             std::string &),
                   override);
  MAKE_CONST_MOCK1(get, RdKafka::Conf::ConfResult(RdKafka::DeliveryReportCb *&),
                   override);
  MAKE_CONST_MOCK1(get, RdKafka::Conf::ConfResult(RdKafka::EventCb *&),
                   override);
  MAKE_CONST_MOCK1(get, RdKafka::Conf::ConfResult(RdKafka::PartitionerCb *&),
                   override);
  MAKE_CONST_MOCK1(
      get, RdKafka::Conf::ConfResult(RdKafka::PartitionerKeyPointerCb *&),
      override);
  MAKE_CONST_MOCK1(get, RdKafka::Conf::ConfResult(RdKafka::SocketCb *&),
                   override);
  MAKE_CONST_MOCK1(get, RdKafka::Conf::ConfResult(RdKafka::OpenCb *&),
                   override);
  MAKE_CONST_MOCK1(get, RdKafka::Conf::ConfResult(RdKafka::RebalanceCb *&),
                   override);
  MAKE_CONST_MOCK1(get, RdKafka::Conf::ConfResult(RdKafka::OffsetCommitCb *&),
                   override);

  MAKE_MOCK3(set,
             RdKafka::Conf::ConfResult(const std::string &,
                                       RdKafka::ConsumeCb *, std::string &),
             override);
  MAKE_MOCK0(dump, std::list<std::string> *(), override);
};
// GCOVR_EXCL_STOP
