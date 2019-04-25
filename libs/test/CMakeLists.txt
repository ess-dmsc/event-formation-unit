set(BitMathTest_SRC
  BitMathTest.cpp
)
  
set(SocketTest_SRC
  SocketTest.cpp
)

include_directories(../../)

create_test_executable(BitMathTest)

create_test_executable(SocketTest)
