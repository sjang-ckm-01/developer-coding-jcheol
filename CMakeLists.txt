cmake_minimum_required(VERSION 3.22)

project(storagecache VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_SANITIZERS "Build with AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
if(ENABLE_SANITIZERS)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=address,undefined)
endif()

option(ENABLE_COVERAGE "Build with gcov instrumentation for coverage reports" OFF)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
  GIT_SHALLOW    TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.9.5
  GIT_SHALLOW    TRUE
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest benchmark)

add_library(storagecache_warnings INTERFACE)
target_compile_options(storagecache_warnings INTERFACE -Wall -Wextra -Werror)

add_library(storagecache_coverage INTERFACE)
if(ENABLE_COVERAGE)
  target_compile_options(storagecache_coverage INTERFACE --coverage -O0 -g)
  target_link_options(storagecache_coverage INTERFACE --coverage)
endif()

add_library(storagecache_lib
    src/storagecache.cpp
)
target_include_directories(storagecache_lib PUBLIC src/include PRIVATE src)
target_link_libraries(storagecache_lib PRIVATE storagecache_warnings
                                              storagecache_coverage)

add_executable(storagecache
    src/main.cpp
)
target_link_libraries(storagecache PRIVATE storagecache_lib storagecache_warnings)

enable_testing()

add_executable(storagecache_test
    tests/storagecache_test.cpp
)
target_link_libraries(storagecache_test PRIVATE gtest_main storagecache_lib
                                              storagecache_warnings
                                              storagecache_coverage)

include(GoogleTest)
gtest_discover_tests(storagecache_test)

add_executable(storagecache_bench
    bench/storagecache_bench.cpp
)
target_link_libraries(storagecache_bench PRIVATE benchmark::benchmark_main
                                                storagecache_lib storagecache_warnings)
target_include_directories(storagecache_bench PRIVATE src)
