cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

include_directories(.. ../..)

# Create an excecutable for the event-builder to process recorded data from a text-file.
find_package(TCLAP)
if (TCLAP_FOUND)
    message(STATUS "TCLAP found")
    set(mbprocesstextfile_INC mbcaen/multiBladeEventBuilder.h mbcaen/TextFile.h mbcaen/dumpEventBuilderInfo.h)
    set(mbprocesstextfile_SRC mbcaen/mbprocesstextfile.cpp mbcaen/multiBladeEventBuilder.cpp mbcaen/TextFile.cpp mbcaen/dumpEventBuilderInfo.cpp)
    create_executable(mbprocesstextfile "")
else (TCLAP_FOUND)
    message(STATUS "TCLAP not found")
endif ()

# Create the tests
enable_testing()

# Common include path for both tests
set(mbtest_INC  mbcaen/multiBladeEventBuilder.h  mbcaen/multiBladeTestData.h)

# Create excecutable for the test of counters
set(mbtestcounter_INC ${mbtest_INC})
set(mbcountertest_SRC mbcaen/multiBladeEventBuilder.cpp mbcaen/multiBladeEventBuilderCounterTest.cpp)
create_test_executable(mbcountertest "")

# Create excecutable for the test of clustering
set(mbclustertest_INC ${MBtest_INC})
set(mbclustertest_SRC mbcaen/multiBladeEventBuilder.cpp mbcaen/multiBladeEventBuilderClusteringTest.cpp)
create_test_executable(mbclustertest "")
