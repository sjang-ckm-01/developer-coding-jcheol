SOURCES := src tests bench

.PHONY: build run test bench fmt fmt-check test-asan coverage clean

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
	find $(SOURCES) -type f \( -name '*.cpp' -o -name '*.h' \) -exec clang-format -i {} +

# Fail if anything is unformatted, without touching the tree.
fmt-check:
	find $(SOURCES) -type f \( -name '*.cpp' -o -name '*.h' \) -exec clang-format --dry-run --Werror {} +

test-asan:
	cmake -B build-asan -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
	cmake --build build-asan --target storagecache_test
	./build-asan/storagecache_test

# Line/branch coverage of src/, measured by the test suite.
coverage:
	cmake -B build-coverage -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
	cmake --build build-coverage --target storagecache_test
	find build-coverage -name '*.gcda' -delete
	./build-coverage/storagecache_test
	gcovr --root . --filter src/ --exclude-throw-branches \
		--exclude-unreachable-branches --print-summary \
		--html-details build-coverage/coverage.html build-coverage

clean:
	rm -rf build build-asan build-coverage
