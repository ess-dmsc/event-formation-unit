
add_subdirectory(geometry)
add_subdirectory(mesytec)
add_subdirectory(generators)
add_subdirectory(reduction)

#=============================================================================
# multigrid detector module and base test
#=============================================================================

set(mgmesytec_SRC MultigridBase.cpp Config.cpp)
set(mgmesytec_INC MultigridBase.h AbstractBuilder.h Config.h)
set(mgmesytec_LIB  MgGeometryLib MgMesytecLib MgReductionLib)
create_module(mgmesytec)

#=============================================================================
# Tests
#=============================================================================

get_filename_component(TEST_JSON_PATH "configs/ILL_mappings.json" DIRECTORY)
set(TEST_JSON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_JSON_PATH}/")

add_subdirectory(geometry/test)
add_subdirectory(mesytec/test)
add_subdirectory(reduction/test)
add_subdirectory(test)
