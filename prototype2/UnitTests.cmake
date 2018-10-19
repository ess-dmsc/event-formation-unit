# Could this be moved to the libs directory somehow ???

#
# Libs unit tests
#

set(BitMathTest_SRC
  ../libs/test/BitMathTest.cpp
)
set(BitMathTest_INC
  ../libs/include/BitMath.h
)
create_test_executable(BitMathTest "")


set(SocketTest_SRC
  ../libs/source/Socket.cpp
  ../libs/test/SocketTest.cpp
)
set(SocketTest_INC
  ../libs/include/Socket.h
)
create_test_executable(SocketTest "")
