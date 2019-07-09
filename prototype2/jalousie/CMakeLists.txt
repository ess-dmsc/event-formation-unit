set(JalReadoutTest_SRC
  ReadoutTest.cpp
  Readout.cpp
  )
set(JalReadoutTest_INC
  Readout.h
  )
create_test_executable(JalReadoutTest SKIP_MEMGRIND)

set(TEST_DATA_PATH "${REFDATA}/jalousie/2019_07_05")
if(EXISTS ${TEST_DATA_PATH})
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
  target_compile_definitions(CdtFileTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
else()
  message(WARNING "*** No jalousie reference data found. Skipping CdtFileTest.")
endif()

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
