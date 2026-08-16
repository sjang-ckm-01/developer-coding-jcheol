#include <memory>

#include "storagecache_internal.h"

namespace storagecache {

StorageCache* createCache(int capacity) {
  if (capacity < 1) {
    return nullptr;
  }

  try {
    auto store = std::make_unique<StorageCache>();
    store->capacity = static_cast<std::size_t>(capacity);
    store->kv.reserve(store->capacity);
    return store.release();
  } catch (...) {
    return nullptr;
  }
}

void freeCache(StorageCache* store) {
  if (store) {
    delete store;
  }
}

int put(StorageCache* store, int key, int value) {
  if (store == nullptr) {
    return -1;
  }

  auto& kv = store->kv;
  auto& lru = store->lru;

  auto kv_it = kv.find(key);
  if (kv_it != kv.end()) {
    lru.splice(lru.end(), lru, kv_it->second);
    kv_it->second->second = value;
    return 0;
  }

  if (kv.size() >= store->capacity) {
    const ListIt victim = lru.begin();
    auto node = kv.extract(victim->first);
    node.key() = key;
    kv.insert(std::move(node));
    victim->first = key;
    victim->second = value;
    lru.splice(lru.end(), lru, victim);
    return 0;
  }

  std::list<Entry> staged;
  staged.emplace_back(key, value);
  kv.emplace(key, staged.begin());
  lru.splice(lru.end(), staged);

  return 0;
}

int get(StorageCache* store, int key) {
  if (store == nullptr) {
    return -1;
  }

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
  if (store == nullptr) {
    return std::vector<int>();
  }

  std::vector<int> v;
  v.reserve(store->lru.size());

  for (auto it = store->lru.rbegin(); it != store->lru.rend(); ++it) {
    v.push_back(it->first);
  }

  return v;
}

}  // namespace storagecache
