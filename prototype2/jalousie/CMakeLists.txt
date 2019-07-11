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

  set(JalBuilderReadoutsTest_SRC
    BuilderReadoutsTest.cpp
    BuilderReadouts.cpp
    Readout.cpp
    CdtFile.cpp
    )
  set(JalBuilderReadoutsTest_INC
    BuilderReadouts.h
    Readout.h
    CdtFile.h
    )
  create_test_executable(JalBuilderReadoutsTest)
  target_compile_definitions(JalBuilderReadoutsTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
else()
  message(WARNING "*** No Jalousie reference data found. Skipping CdtFileTest.")
  message(WARNING "*** No Jalousie reference data found. Skipping JalBuilderReadoutsTest.")
endif()

set(udpgen_jalousie_SRC
  ../generators/generator.cpp
  CdtFile.cpp
  Readout.cpp
  )
set(udpgen_jalousie_INC
  CdtFile.h
  Readout.h
  )
create_executable(udpgen_jalousie)
target_compile_definitions(udpgen_jalousie PUBLIC GENERATOR_JALOUSIE_CDT)

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

add_subdirectory(generators)
