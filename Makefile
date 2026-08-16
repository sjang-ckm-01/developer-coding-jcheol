SOURCES := storagecache tests bench

.PHONY: build test bench fmt fmt-check test-asan coverage clean

build:
	cmake -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build

test: build
	ctest --test-dir build --output-on-failure

bench: build
	./build/bench/storagecache_bench

# Reformat every C++ source in place.
fmt:
	find $(SOURCES) -type f \( -name '*.cpp' -o -name '*.h' \) -exec clang-format -i {} +

# Fail if anything is unformatted, without touching the tree.
fmt-check:
	find $(SOURCES) -type f \( -name '*.cpp' -o -name '*.h' \) -exec clang-format --dry-run --Werror {} +

test-asan:
	cmake -B build-asan -DCMAKE_BUILD_TYPE=Debug -DSTORAGECACHE_ENABLE_SANITIZERS=ON
	cmake --build build-asan --target storagecache_test
	ctest --test-dir build-asan --output-on-failure

# Line/branch coverage of the library, measured by the test suite.
coverage:
	cmake -B build-coverage -DCMAKE_BUILD_TYPE=Debug -DSTORAGECACHE_ENABLE_COVERAGE=ON
	cmake --build build-coverage --target storagecache_test
	find build-coverage -name '*.gcda' -delete
	ctest --test-dir build-coverage --output-on-failure
	gcovr --root . --filter storagecache/ --exclude-throw-branches \
		--exclude-unreachable-branches --print-summary \
		--html-details build-coverage/coverage.html build-coverage

clean:
	rm -rf build build-asan build-coverage
