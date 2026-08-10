#include <benchmark/benchmark.h>

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

#include "storagecache_internal.h"

// [0, count) 범위의 서로 다른 키를 무작위 순서로 생성.
// rand() % count 는 중복이 생겨 실제 원소 수가 count 보다 적어진다.
static std::vector<int> generate_keys(int count, unsigned seed) {
  std::vector<int> v(count);
  std::iota(v.begin(), v.end(), 0);
  std::shuffle(v.begin(), v.end(), std::mt19937(seed));
  return v;
}

// 저장소 크기 N과 무관하게 put() 호출 횟수를 고정한다.
static constexpr int kProbes = 100'000;

// eviction 경로 측정: 캐시를 n개로 채워 포화시킨 뒤,
// 캐시에 없는 키만 넣어 매 호출이 축출 1회 + 삽입 1회를 하게 한다.
static void BM_put(benchmark::State& state) {
  const int n = static_cast<int>(state.range(0));

  auto store = storagecache::createCache(n);
  const auto keys = generate_keys(n, 12345);
  for (int k : keys) {
    storagecache::put(store, k, k);
  }

  // 캐시에는 [0, n) 이 들어있으므로 축출을 강제하려면 그 밖의 키가 필요하다.
  // 도메인 폭을 max(n, kProbes)로 잡아야 버킷이 배열 전체에 흩어진다.
  // (libstdc++ hash<int>는 항등 함수라 좁은 도메인은 버킷도 좁게 뭉친다)
  const int domain = std::max(n, kProbes);
  const auto fresh = generate_keys(domain, 67890);

  // fresh는 서로 다른 값이라 kProbes번 모두 새 키 = 100% 축출 분기.
  int i = 0;
  for (auto _ : state) {
    storagecache::put(store, n + fresh[i], i);
    ++i;
  }

  // store->kv 를 읽는 코드는 전부 freeCache 앞에 있어야 한다.
  state.counters["fill_ratio"] = static_cast<double>(store->kv.size()) / n;
  state.counters["load_factor"] = store->kv.load_factor();
  if (store->kv.size() != static_cast<size_t>(n)) {
    state.SkipWithError("cache not saturated: eviction path never ran");
  }

  storagecache::freeCache(store);
  state.SetItemsProcessed(state.iterations());
  state.SetComplexityN(n);
}
BENCHMARK(BM_put)
    ->RangeMultiplier(10)
    ->Range(1000, 10'000'000)
    ->Iterations(kProbes)
    ->Complexity();

// hit 경로 측정: 캐시를 n개로 채우고, 그 안에 있는 키만 조회한다.
static void BM_get(benchmark::State& state) {
  const int n = static_cast<int>(state.range(0));

  auto store = storagecache::createCache(n);
  const auto keys = generate_keys(n, 12345);
  for (int k : keys) {
    storagecache::put(store, k, k);
  }

  // 삽입 순서와 조회 순서의 상관관계를 없앤다.
  const auto probes = generate_keys(n, 67890);

  size_t i = 0;
  for (auto _ : state) {
    int val = storagecache::get(store, probes[i]);
    benchmark::DoNotOptimize(val);
    if (++i == probes.size()) i = 0;
  }

  storagecache::freeCache(store);

  state.SetItemsProcessed(state.iterations());
  state.SetComplexityN(n);
}
BENCHMARK(BM_get)
    ->RangeMultiplier(10)
    ->Range(1000, 10'000'000)
    ->Iterations(100'000)
    ->Complexity();
