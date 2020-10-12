
add_subdirectory(generators)
add_subdirectory(test)

include_directories(.)

#=============================================================================
# jalousie detector module 
#=============================================================================

set(jalousie_common_inc Config.h Readout.h SumoMappings.h rapidcsv.h)
set(jalousie_common_src Config.cpp Readout.cpp SumoMappings.cpp)

set(jalousie_INC JalousieBase.h ${jalousie_common_inc})
set(jalousie_SRC JalousieBase.cpp Jalousie.cpp ${jalousie_common_src})
create_module(jalousie)
