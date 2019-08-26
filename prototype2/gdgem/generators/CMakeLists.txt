#=============================================================================
# Raw evenlet generator (h5)
#=============================================================================

#
set(nmxgen_hits_SRC
  ../../generators/generator.cpp
    ReaderHits.cpp
    )
set(nmxgen_hits_INC
    ReaderHits.h
    )
create_executable(nmxgen_hits)
target_compile_definitions(nmxgen_hits PUBLIC GENERATOR_GDGEM_HITS)

#
set(nmxgen_readouts_SRC
  ../../generators/generator.cpp
  ReaderReadouts.cpp
  )
set(nmxgen_readouts_INC
  ReaderReadouts.h
  )
create_executable(nmxgen_readouts)
target_compile_definitions(nmxgen_readouts PUBLIC GENERATOR_GDGEM_READOUTS)
