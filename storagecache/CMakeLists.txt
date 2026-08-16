add_library(storagecache
    src/storagecache.cpp
)
add_library(storagecache::storagecache ALIAS storagecache)

target_include_directories(storagecache
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(storagecache PUBLIC cxx_std_17)

storagecache_apply_build_options(storagecache)
