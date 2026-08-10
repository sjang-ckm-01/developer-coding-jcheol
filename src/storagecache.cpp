#include <cstdio>
#include <format>

#include "storagecache_internal.h"

template <class... Args>
void println(std::format_string<Args...> fmt, Args&&... args) {
  std::string s = std::format(fmt, std::forward<Args>(args)...);
  s.push_back('\n');
  std::fwrite(s.data(), 1, s.size(), stdout);
};

namespace storagecache {

StorageCache* createCache(int capacity) {
  StorageCache* store = new StorageCache;
  store->capacity = capacity;
  store->kv.reserve(capacity);

  return store;
}

void freeCache(StorageCache* store) {
  if (store) {
    delete store;
  }
}

void put(StorageCache* store, int key, int value) {
  auto& kv = store->kv;
  auto& lru = store->lru;
  auto kv_it = kv.find(key);

  if (kv_it != kv.end()) {
    lru.erase(kv_it->second);
  } else if (kv.size() >= store->capacity) {
    auto it = lru.front();
    kv.erase(it.first);
    lru.pop_front();
  }

  kv[key] = lru.emplace(lru.end(), std::make_pair(key, value));
}

int get(StorageCache* store, int key) {
  const auto& kv = store->kv;
  auto& lru = store->lru;

  const auto& it = kv.find(key);
  if (it == kv.end()) {
    return -1;
  }

  lru.splice(lru.end(), lru, it->second);

  return it->second->second;
}

std::vector<int> get_orders(const StorageCache* store) {
  std::vector<int> v;
  v.reserve(store->capacity);

  for (auto it = store->lru.rbegin(); it != store->lru.rend(); ++it) {
    v.push_back(it->first);
  }

  return v;
}

}  // namespace storagecache
