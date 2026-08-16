FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
  GIT_SHALLOW    TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(storagecache_test
    storagecache_test.cpp
)
target_link_libraries(storagecache_test PRIVATE
    storagecache::storagecache
    GTest::gtest_main
)

storagecache_apply_build_options(storagecache_test)

include(GoogleTest)
gtest_discover_tests(storagecache_test)
