#pragma once

#include <vector>

namespace storagecache {

struct StorageCache;

// assignment mandatory APIs
StorageCache* createCache(int capacity);
void freeCache(StorageCache* store);
void put(StorageCache* store, int key, int value);
int get(StorageCache* store, int key);

// additional APIs for testing
std::vector<int> get_orders(const StorageCache* store);

}  // namespace storagecache
