FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.9.5
  GIT_SHALLOW    TRUE
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

add_executable(storagecache_bench
    storagecache_bench.cpp
)
target_link_libraries(storagecache_bench PRIVATE
    storagecache::storagecache
    benchmark::benchmark_main
)

# The benchmark reads bucket counts and fill ratios straight off the internals.
target_include_directories(storagecache_bench PRIVATE
    ${PROJECT_SOURCE_DIR}/storagecache/src
)

storagecache_apply_build_options(storagecache_bench)
