/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Producer.h>
#include <cstring>
#include <dlfcn.h>
#include <librdkafka/rdkafkacpp.h>
#include <test/TestBase.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
int fail = -1; // Dont fail

// Create pointers to the retuned objects
typedef RdKafka::Conf *(*pcreate)(RdKafka::Conf::ConfType);
typedef RdKafka::Producer *(*pcreateprod)(RdKafka::Conf *, std::string &);
typedef RdKafka::Topic *(*pcreatetopic)(RdKafka::Handle *, std::string const &,
                                        RdKafka::Conf *, std::string &);

/**Attmpt to load primary and alternatively secondary symbols using dlsym()
 * Created to cover different compilations of librdkafka (old and new abis)
 * Used for forcing external functions to fail for unit testing purposes
 * exit if none of the symbols can be loaded.
**/
void * loadsyms(const char * primary, const char * secondary) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

  void * vpsym = dlsym(RTLD_NEXT, primary);

  if (vpsym == NULL) {
    printf("Could not load primary symbol: %s\n", primary);
    printf("Trying secondary...\n");

    dlerror();
    vpsym = dlsym(RTLD_NEXT, secondary);
    if (vpsym == NULL) {
      printf("Could not load secondary symbol: %s\n", secondary);
      printf("Error stubbing kafka functions\n");
      exit(1);
    }
  }
#pragma GCC diagnostic pop
  return vpsym;
}


/** Intercept RdKafka::Conf::create() */
RdKafka::Conf *RdKafka::Conf::create(ConfType type) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
  pcreate real_create = (pcreate)loadsyms(
      "_ZN7RdKafka4Conf6createENS0_8ConfTypeE",
      "_ZN7RdKafka4Conf6createENS0_8ConfTypeE"); // nm -C
#pragma GCC diagnostic pop

  if (fail != -1 && type == fail) {
    printf("Forcing RdKafka::Conf::create() to fail\n");
    return nullptr;
  } else {
    return real_create(type);
  }
}


/** Intercept RdKafka::Producer::create() */
RdKafka::Producer *RdKafka::Producer::create(RdKafka::Conf *type,
                                             std::string &str) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
  pcreateprod real_create = (pcreateprod)loadsyms(
      "_ZN7RdKafka8Producer6createEPNS_4ConfERSs",
      "_ZN7RdKafka8Producer6createEPNS_4ConfERNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE"); // nm -C librdkafka.a
#pragma GCC diagnostic pop

  if (fail == 777) {
    printf("Forcing RdKafka::Producer::create() to fail\n");
    return nullptr;
  } else {
    return real_create(type, str);
  }
}


/** Intercept RdKafka::Topic::create() */
RdKafka::Topic *RdKafka::Topic::create(RdKafka::Handle *handle,
                                       std::string const &topic,
                                       RdKafka::Conf *conf, std::string &str) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
  pcreatetopic real_create = (pcreatetopic)loadsyms(
      "_ZN7RdKafka5Topic6createEPNS_6HandleERKSsPNS_4ConfERSs",
      "_ZN7RdKafka5Topic6createEPNS_6HandleERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEPNS_4ConfERS8_");
#pragma GCC diagnostic pop

  if (fail == 888) {
    printf("Forcing RdKafka::Topic::create() to fail\n");
    return nullptr;
  } else {
    return real_create(handle, topic, conf, str);
  }
}

//
// Google test code below
//

class ProducerTest : public TestBase {
  virtual void SetUp() { fail = -1; }
  virtual void TearDown() {}
};

TEST_F(ProducerTest, ConstructorOK) {
  Producer prod{"nobroker", "notopic"};
  int ret = prod.produce(0, 0);
  ASSERT_EQ(ret, RdKafka::ERR_NO_ERROR);
  ASSERT_EQ(prod.stats.dr_errors, 0);
  ASSERT_EQ(prod.stats.dr_noerrors, 0);
  ASSERT_EQ(prod.stats.ev_errors, 0);
  ASSERT_EQ(prod.stats.ev_others, 0);
  ASSERT_EQ(prod.stats.produce_fails, 0);
}

TEST_F(ProducerTest, CreateConfGlobalFail) {
  fail = RdKafka::Conf::CONF_GLOBAL;
  Producer prod{"nobroker", "notopic"};
  int ret = prod.produce(0, 0);
  ASSERT_EQ(ret, RdKafka::ERR_UNKNOWN);
}

TEST_F(ProducerTest, CreateConfTopicFail) {
  fail = RdKafka::Conf::CONF_TOPIC;
  Producer prod{"nobroker", "notopic"};
  int ret = prod.produce(0, 0);
  ASSERT_EQ(ret, RdKafka::ERR_UNKNOWN);
}

TEST_F(ProducerTest, CreateProducerFail) {
  fail = 777;
  Producer prod{"nobroker", "notopic"};
  int ret = prod.produce(0, 0);
  ASSERT_EQ(ret, RdKafka::ERR_UNKNOWN);
}

TEST_F(ProducerTest, CreateTopicFail) {
  fail = 888;
  Producer prod{"nobroker", "notopic"};
  int ret = prod.produce(0, 0);
  ASSERT_EQ(ret, RdKafka::ERR_UNKNOWN);

}

TEST_F(ProducerTest, ProducerFail) {
  Producer prod{"nobroker", "notopic"};
  int ret = prod.produce((char *)10000, 100000000);
  ASSERT_EQ(ret, RdKafka::ERR_MSG_SIZE_TOO_LARGE);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#pragma GCC diagnostic pop
