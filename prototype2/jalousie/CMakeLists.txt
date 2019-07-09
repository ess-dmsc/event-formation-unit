set(JalReadoutTest_SRC
  ReadoutTest.cpp
  Readout.cpp
  )
set(JalReadoutTest_INC
  Readout.h
  )
create_test_executable(JalReadoutTest SKIP_MEMGRIND)

set(CdtFileTest_SRC
  CdtFileTest.cpp
  CdtFile.cpp
  Readout.cpp
  )
set(CdtFileTest_INC
  CdtFile.h
  Readout.h
  )
create_test_executable(CdtFileTest)


set(jalousie_common_inc
  JalousieBase.h
  )
set(jalousie_common_src
  JalousieBase.cpp
  )

set(jalousie_INC
  ${jalousie_common_inc}
)
set(jalousie_SRC
  Jalousie.cpp
  ${jalousie_common_src}
)
create_module(jalousie)
