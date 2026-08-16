#include "storagecache.h"

#include <gtest/gtest.h>

using namespace storagecache;

TEST(StorageCacheTest, CreateStartsEmpty) {
  StorageCache* store = createCache(4);
  ASSERT_NE(store, nullptr);

  EXPECT_EQ(get(store, 1), -1);
  EXPECT_EQ(get(store, 2), -1);
  EXPECT_EQ(get(store, 99), -1);

  freeCache(store);
}

TEST(StorageCacheTest, FreeAcceptsNullptr) { freeCache(nullptr); }

TEST(StorageCacheTest, CreateRejectsNonPositiveCapacity) {
  EXPECT_EQ(createCache(0), nullptr);
  EXPECT_EQ(createCache(-1), nullptr);
}

TEST(StorageCacheTest, ApisTolerateNullStore) {
  StorageCache* store = createCache(-1);
  ASSERT_EQ(store, nullptr);

  EXPECT_EQ(put(store, 1, 100), -1);
  EXPECT_EQ(get(store, 1), -1);
  EXPECT_TRUE(get_orders(store).empty());

  freeCache(store);
}

TEST(StorageCacheTest, CreateAcceptsSmallestValidCapacity) {
  StorageCache* store = createCache(1);
  ASSERT_NE(store, nullptr);

  put(store, 1, 100);
  put(store, 2, 200);

  EXPECT_EQ(get(store, 1), -1);
  EXPECT_EQ(get(store, 2), 200);

  freeCache(store);
}

TEST(StorageCacheTest, FollowsAssignmentScenario) {
  // 1. 용량 4로 저장소 초기화
  StorageCache* store = createCache(4);

  // 2. 데이터 순차 저장
  put(store, 1, 100);
  // 현재 최신순: 1

  put(store, 2, 200);
  // 현재 최신순: 2 -> 1
  EXPECT_EQ(get_orders(store), (std::vector{2, 1}));

  put(store, 3, 300);
  // 현재 최신순: 3 -> 2 -> 1
  EXPECT_EQ(get_orders(store), (std::vector{3, 2, 1}));

  put(store, 4, 400);  // 저장소 가득 참
  // 현재 최신순: 4 -> 3 -> 2 -> 1 (가장 오른쪽 '1'이 삭제 후보)
  EXPECT_EQ(get_orders(store), (std::vector{4, 3, 2, 1}));

  // 3. 데이터 조회 (우선순위 변경 발생)
  EXPECT_EQ(get(store, 1),
            100);  // 반환값: 100
                   // 1번을 조회했으므로 가장 최신 위치로 이동합니다.
                   // 현재 최신순: 1 -> 4 -> 3 -> 2 (이제 '2'가 삭제 후보)
  EXPECT_EQ(get_orders(store), (std::vector{1, 4, 3, 2}));

  // 4. 용량 초과 데이터 저장 (Eviction 발생)
  put(store, 5, 500);  // 새로운 Key 등장! 용량(4) 초과!
                       // 가장 뒤에 있는(오래된) 2번을 제거하고, 5번을
                       // 저장합니다. 현재 최신순: 5 -> 1 -> 4 -> 3
  EXPECT_EQ(get_orders(store), (std::vector{5, 1, 4, 3}));

  // 5. 삭제된 데이터 확인
  EXPECT_EQ(get(store, 2), -1);  // 반환값: -1 (찾을 수 없음)

  // 6. 연속 저장 및 조회
  put(store, 6, 600);  // 용량 초과!
                       // 가장 뒤에 있는 3번을 제거하고, 6번을 저장합니다.
                       // 현재 최신순: 6 -> 5 -> 1 -> 4
  EXPECT_EQ(get_orders(store), (std::vector{6, 5, 1, 4}));

  EXPECT_EQ(get(store, 3), -1);   // 반환값: -1 (제거됨)
  EXPECT_EQ(get(store, 1), 100);  // 반환값: 100 (아직 살아있음)
  EXPECT_EQ(get_orders(store),
            (std::vector{1, 6, 5, 4}));  // 현재 최신순: 1 -> 6 -> 5 -> 4
  EXPECT_EQ(get(store, 5), 500);         // 반환값: 500
  EXPECT_EQ(get_orders(store),
            (std::vector{5, 1, 6, 4}));  // 현재 최신순: 5 -> 1 -> 6 -> 4

  // 7. 메모리 정리
  freeCache(store);
}

TEST(StorageCacheTest, PutStoresAndOverwritesValues) {
  StorageCache* store = createCache(4);

  put(store, 1, 1);
  EXPECT_EQ(get(store, 1), 1);
  EXPECT_EQ(get(store, 2), -1);

  put(store, 1, 2);
  EXPECT_EQ(get(store, 1), 2);

  put(store, 2, 3);
  EXPECT_EQ(get(store, 1), 2);
  EXPECT_EQ(get(store, 2), 3);

  freeCache(store);
}

class FilledCacheTest : public ::testing::Test {
 protected:
  void SetUp() override {
    store = createCache(4);
    ASSERT_NE(store, nullptr);

    put(store, 1, 1);
    put(store, 2, 2);
    put(store, 3, 3);
    put(store, 4, 4);
  }

  void TearDown() override { freeCache(store); }

  StorageCache* store = nullptr;
};

TEST_F(FilledCacheTest, EvictsLeastRecentlyUsedWhenFull) {
  EXPECT_EQ(get(store, 1), 1);
  EXPECT_EQ(get(store, 2), 2);
  EXPECT_EQ(get(store, 3), 3);
  EXPECT_EQ(get(store, 4), 4);

  put(store, 5, 5);
  EXPECT_EQ(get(store, 1), -1);
  EXPECT_EQ(get(store, 2), 2);
  EXPECT_EQ(get(store, 3), 3);
  EXPECT_EQ(get(store, 4), 4);
  EXPECT_EQ(get(store, 5), 5);
}

TEST_F(FilledCacheTest, OrderReportAgreesWithEviction) {
  get(store, 2);

  std::vector<int> order = get_orders(store);
  ASSERT_EQ(order.size(), 4u);
  const int claimed_oldest = order.back();

  put(store, 99, 99);

  EXPECT_EQ(get(store, claimed_oldest), -1);
  for (std::size_t i = 0; i + 1 < order.size(); ++i) {
    EXPECT_EQ(get(store, order[i]), order[i]);
  }
}

class StorageCacheUpdateTest : public FilledCacheTest,
                               public ::testing::WithParamInterface<int> {};

TEST_P(StorageCacheUpdateTest, UpdatingExistingKeyDoesNotEvict) {
  const int key = GetParam();

  EXPECT_EQ(get(store, 1), 1);
  EXPECT_EQ(get(store, 2), 2);
  EXPECT_EQ(get(store, 3), 3);
  EXPECT_EQ(get(store, 4), 4);

  put(store, key, 101);

  for (int k = 1; k <= 4; ++k) {
    EXPECT_EQ(get(store, k), k == key ? 101 : k);
  }
}

TEST_P(StorageCacheUpdateTest, UpdatingExistingKeyMovesItToMostRecent) {
  const int key = GetParam();

  put(store, key, 101);

  std::vector<int> expected{key};
  for (int k = 4; k >= 1; --k) {
    if (k != key) expected.push_back(k);
  }
  EXPECT_EQ(get_orders(store), expected);

  put(store, 5, 5);
  EXPECT_EQ(get(store, key), 101);
}

INSTANTIATE_TEST_SUITE_P(AtEachLruPosition, StorageCacheUpdateTest,
                         ::testing::Values(1, 2, 3, 4));
