sources := "src tests bench"

build:
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build

run: build
    ./build/storagecache

test: build
    ./build/storagecache_test

bench: build
    ./build/storagecache_bench

# Reformat every C++ source in place.
fmt:
    find {{sources}} -type f \( -name '*.cpp' -o -name '*.h' \) -exec clang-format -i {} +

# Fail if anything is unformatted, without touching the tree.
fmt-check:
    find {{sources}} -type f \( -name '*.cpp' -o -name '*.h' \) -exec clang-format --dry-run --Werror {} +

test-asan:
    cmake -B build-asan -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
    cmake --build build-asan --target storagecache_test
    ./build-asan/storagecache_test

clean:
    rm -rf build build-asan
