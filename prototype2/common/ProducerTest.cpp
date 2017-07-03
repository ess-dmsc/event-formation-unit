/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Producer.h>
#include <test/TestBase.h>
#include <cstring>
#include <librdkafka/rdkafkacpp.h>
#include <dlfcn.h>

int fail = -1; // Dont fail

typedef RdKafka::Conf *(* pcreate) (RdKafka::Conf::ConfType);

RdKafka::Conf * RdKafka::Conf::create(ConfType type) {
  pcreate real_create;
  real_create  = (pcreate)dlsym(RTLD_NEXT, "_ZN7RdKafka4Conf6createENS0_8ConfTypeE"); // nm -C
  if (fail != -1 && type == fail) {
    printf("Forcing RdKafka::Conf::create() to fail\n");
    return nullptr;
  } else {
    return real_create(type);
  }
}

typedef RdKafka::Producer *(* pcreateprod) (RdKafka::Conf*, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >&);
RdKafka::Producer * RdKafka::Producer::create(RdKafka::Conf* type, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >& str) {
  pcreateprod real_create;
  real_create  = (pcreateprod)dlsym(RTLD_NEXT, "_ZN7RdKafka8Producer6createEPNS_4ConfERNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE"); // nm -C
  if (fail == 777) {
    printf("Forcing RdKafka::Producer::create() to fail\n");
    return nullptr;
  } else {
    return real_create(type, str);
  }
}

typedef RdKafka::Topic *(* pcreatetopic) (RdKafka::Handle*, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, RdKafka::Conf*, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >&);
RdKafka::Topic * RdKafka::Topic::create(RdKafka::Handle* handle, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const& topic, RdKafka::Conf* conf, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >& str) {
  pcreatetopic real_create;
  real_create  = (pcreatetopic)dlsym(RTLD_NEXT, "_ZN7RdKafka5Topic6createEPNS_6HandleERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEPNS_4ConfERS8_"); // nm -C
  if (fail == 888) {
    printf("Forcing RdKafka::Topic::create() to fail\n");
    return nullptr;
  } else {
    return real_create(handle, topic, conf, str);
  }
}

class ProducerTest : public TestBase {
    virtual void SetUp() { fail = -1; }
    virtual void TearDown() {}
};

TEST_F(ProducerTest, ConstructorOK) {
  Producer prod{"nobroker", "notopic"};
  auto ret = prod.produce(0, 0);
  ASSERT_EQ(ret, RdKafka::ERR_NO_ERROR);
}

TEST_F(ProducerTest, CreateConfGlobalFail) {
  fail = RdKafka::Conf::CONF_GLOBAL;
  Producer prod{"nobroker", "notopic"};
}

TEST_F(ProducerTest, CreateConfTopicFail) {
  fail = RdKafka::Conf::CONF_TOPIC;
  Producer prod{"nobroker", "notopic"};
}

TEST_F(ProducerTest, CreateProducerFail) {
  fail = 777;
  Producer prod{"nobroker", "notopic"};
  fail = 0;
}

TEST_F(ProducerTest, CreateTopicFail) {
  fail = 888;
  Producer prod{"nobroker", "notopic"};
  fail = 0;
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
