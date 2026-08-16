#pragma once

#include <cstddef>
#include <list>
#include <unordered_map>
#include <utility>

#include "storagecache.h"

namespace storagecache {

using Entry = std::pair<int, int>;
using ListIt = std::list<Entry>::iterator;

struct StorageCache {
  std::size_t capacity;
  std::list<Entry> lru;
  std::unordered_map<int, ListIt> kv;
};

}  // namespace storagecache
