# Could this be moved to the libs directory somehow ???

#
# Libs unit tests
#

set(CounterTest_SRC
  ../libs/test/Counter/testmain.cpp
  ../libs/test/Counter/CounterTest.cpp)
set(CounterTest_INC
  ../libs/include/Counter.h)
create_test_executable(CounterTest "")

set(StatCounterTest_SRC
  ../libs/test/Counter/testmain.cpp
  ../libs/test/Counter/StatCounterTest.cpp)
set(StatCounterTest_INC
  ../libs/include/StatCounter.h)
create_test_executable(StatCounterTest "")


