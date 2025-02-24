add_compile_options(-Wno-everything)

# Build doctest
add_subdirectory(googletest)

# Build magic_enum
add_subdirectory(magic_enum)

# Build spdlog
add_subdirectory(spdlog)

set(TLSF_DIR tlsf)
add_library(
        tlsf
        ${TLSF_DIR}/tlsf.cpp
)

# Build roaring
add_subdirectory(croaring)

# Build mlas
add_subdirectory(mlas)

# Build concurrentqueue
add_subdirectory(concurrentqueue)

#add_library(libhv INTERFACE)
#target_include_directories(libhv INTERFACE libhv/include)

# Build newpfor
add_subdirectory(newpfor)

# Build zsv
add_subdirectory(zsv)

################################################################################
### Highway library
################################################################################

set(BUILD_TESTING OFF CACHE INTERNAL "" FORCE)
add_subdirectory(highway)

################################################################################
### sse2neon
### need this after highway and before simdcomp
################################################################################

add_library(sse2neon INTERFACE)

if (ARCH_AARCH64)
  set(SSE2NEON_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/sse2neon)

  target_include_directories(
    sse2neon
    SYSTEM
    INTERFACE ${SSE2NEON_ROOT}
  )
endif ()

################################################################################
### OpenFST
################################################################################

add_subdirectory(openfst)

################################################################################
### iresearch
################################################################################

add_subdirectory(iresearch)

################################################################################
### thrift
################################################################################
add_subdirectory(thrift)

################################################################################
### vespalib
################################################################################
add_subdirectory(vespalib)
