cmake_minimum_required(VERSION 3.22)

project(storagecache
  VERSION 1.0.0
  DESCRIPTION "Fixed-capacity LRU cache"
  LANGUAGES CXX
)

option(STORAGECACHE_BUILD_TESTS "Build the test suite" ${PROJECT_IS_TOP_LEVEL})
option(STORAGECACHE_BUILD_BENCH "Build the benchmarks" ${PROJECT_IS_TOP_LEVEL})
option(STORAGECACHE_ENABLE_SANITIZERS "Build with AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
option(STORAGECACHE_ENABLE_COVERAGE "Build with gcov instrumentation for coverage reports" OFF)

if(STORAGECACHE_ENABLE_SANITIZERS)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=address,undefined)
endif()

function(storagecache_apply_build_options target)
  target_compile_options(${target} PRIVATE -Wall -Wextra -Werror)
  if(STORAGECACHE_ENABLE_COVERAGE)
    target_compile_options(${target} PRIVATE --coverage -O0 -g)
    target_link_options(${target} PRIVATE --coverage)
  endif()
endfunction()

add_subdirectory(storagecache)

if(STORAGECACHE_BUILD_TESTS OR STORAGECACHE_BUILD_BENCH)
  include(FetchContent)
endif()

if(STORAGECACHE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(STORAGECACHE_BUILD_BENCH)
  add_subdirectory(bench)
endif()
