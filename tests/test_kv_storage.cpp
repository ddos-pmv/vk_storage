#include <gtest/gtest.h>
#include <vk/storage.h>

#include <chrono>
#include <memory>
#include <thread>

// Базовый класс для тестов с обычными часами
class KVStorageTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std::vector<std::tuple<std::string, std::string, uint32_t>> initial_data = {
        {"key1", "value1", 0},   // без TTL
        {"key2", "value2", 10},  // TTL 10 секунд
        {"key3", "value3", 5}    // TTL 5 секунд
    };
    storage_ = std::make_unique<vk::KVStorage<std::chrono::steady_clock>>(
        std::span(initial_data));
  }

  std::unique_ptr<vk::KVStorage<std::chrono::steady_clock>> storage_;
};

// Тесты базовой функциональности
TEST_F(KVStorageTest, BasicGet) {
  auto value = storage_->get("key1");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, "value1");

  auto missing = storage_->get("missing_key");
  EXPECT_FALSE(missing.has_value());
}

TEST_F(KVStorageTest, BasicSet) {
  storage_->set("new_key", "new_value", 0);
  auto value = storage_->get("new_key");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, "new_value");
}

TEST_F(KVStorageTest, UpdateExisting) {
  storage_->set("key1", "updated_value", 0);
  auto value = storage_->get("key1");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, "updated_value");
}

TEST_F(KVStorageTest, UpdateExistingWithTTL) {
  // Обновляем ключ без TTL, добавляя TTL
  storage_->set("key1", "value_with_ttl", 3600);
  auto value = storage_->get("key1");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, "value_with_ttl");
}

TEST_F(KVStorageTest, Remove) {
  EXPECT_TRUE(storage_->remove("key1"));
  EXPECT_FALSE(storage_->get("key1").has_value());
  EXPECT_FALSE(storage_->remove("key1"));  // уже удален
}

TEST_F(KVStorageTest, RemoveNonExistent) {
  EXPECT_FALSE(storage_->remove("non_existent_key"));
}

TEST_F(KVStorageTest, GetManySorted) {
  storage_->set("a_key", "a_value", 0);
  storage_->set("z_key", "z_value", 0);

  auto results = storage_->getManySorted("", 10);

  EXPECT_GE(results.size(), 3);
  // Проверяем что результаты отсортированы
  for (size_t i = 1; i < results.size(); ++i) {
    EXPECT_LE(results[i - 1].first, results[i].first);
  }
}

TEST_F(KVStorageTest, GetManySortedWithStartKey) {
  storage_->set("aaa", "value_aaa", 0);
  storage_->set("bbb", "value_bbb", 0);
  storage_->set("ccc", "value_ccc", 0);

  auto results = storage_->getManySorted("key2", 10);

  EXPECT_GE(results.size(), 1);
  // Первый результат должен быть >= "key2"
  if (!results.empty()) {
    EXPECT_GE(results[0].first, "key2");
  }
}

TEST_F(KVStorageTest, GetManySortedCount) {
  storage_->set("a1", "v1", 0);
  storage_->set("a2", "v2", 0);
  storage_->set("a3", "v3", 0);
  storage_->set("a4", "v4", 0);

  auto results = storage_->getManySorted("a", 2);
  EXPECT_EQ(results.size(), 2);
}

TEST_F(KVStorageTest, EmptyStorage) {
  vk::KVStorage<std::chrono::steady_clock> empty_storage(
      std::span<std::tuple<std::string, std::string, uint32_t>>{});

  EXPECT_FALSE(empty_storage.get("any_key").has_value());
  EXPECT_FALSE(empty_storage.remove("any_key"));

  auto results = empty_storage.getManySorted("", 10);
  EXPECT_TRUE(results.empty());

  EXPECT_FALSE(empty_storage.removeOneExpiredEntry().has_value());
}

// Mock Clock для тестирования TTL
class MockClock {
 public:
  using time_point = std::chrono::steady_clock::time_point;

  MockClock()
      : time_ptr_(
            std::make_shared<time_point>(std::chrono::steady_clock::now())) {}

  time_point now() const { return *time_ptr_; }

  void advance(std::chrono::seconds seconds) { *time_ptr_ += seconds; }

  void set_time(time_point time) { *time_ptr_ = time; }

 private:
  std::shared_ptr<time_point> time_ptr_;
};

// Тесты TTL функциональности
class KVStorageTTLTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std::vector<std::tuple<std::string, std::string, uint32_t>> initial_data = {
        {"persistent", "value", 0},  // без TTL
        {"short_ttl", "value", 5},   // 5 секунд
        {"long_ttl", "value", 3600}  // 1 час
    };
    storage_ = std::make_unique<vk::KVStorage<MockClock>>(
        std::span(initial_data), clock_);
  }

  MockClock clock_;
  std::unique_ptr<vk::KVStorage<MockClock>> storage_;
};

TEST_F(KVStorageTTLTest, TTLExpiration) {
  // Изначально все ключи доступны
  EXPECT_TRUE(storage_->get("short_ttl").has_value());
  EXPECT_TRUE(storage_->get("long_ttl").has_value());
  EXPECT_TRUE(storage_->get("persistent").has_value());

  // Продвигаем время на 6 секунд
  clock_.advance(std::chrono::seconds(6));

  // short_ttl должен истечь
  EXPECT_FALSE(storage_->get("short_ttl").has_value());
  EXPECT_TRUE(storage_->get("long_ttl").has_value());
  EXPECT_TRUE(storage_->get("persistent").has_value());
}

TEST_F(KVStorageTTLTest, RemoveExpiredEntry) {
  // Продвигаем время чтобы short_ttl истек
  clock_.advance(std::chrono::seconds(6));

  auto expired = storage_->removeOneExpiredEntry();
  ASSERT_TRUE(expired.has_value());
  EXPECT_EQ(expired->first, "short_ttl");
  EXPECT_EQ(expired->second, "value");

  // Второй вызов не должен найти истекших записей (пока что)
  auto no_more_expired = storage_->removeOneExpiredEntry();

  // Продвигаем время еще больше
  clock_.advance(std::chrono::seconds(3600));

  // Теперь должен истечь long_ttl
  auto expired2 = storage_->removeOneExpiredEntry();
  ASSERT_TRUE(expired2.has_value());
  EXPECT_EQ(expired2->first, "long_ttl");

  // persistent записи не истекают
  EXPECT_TRUE(storage_->get("persistent").has_value());
}

TEST_F(KVStorageTTLTest, UpdateTTL) {
  storage_->set("short_ttl", "new_value", 3600);

  clock_.advance(std::chrono::seconds(6));

  auto value = storage_->get("short_ttl");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, "new_value");
}

TEST_F(KVStorageTTLTest, RemoveTTL) {
  storage_->set("short_ttl", "persistent_value", 0);

  clock_.advance(std::chrono::seconds(10000));

  auto value = storage_->get("short_ttl");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, "persistent_value");
}

TEST_F(KVStorageTTLTest, GetManySortedSkipsExpired) {
  clock_.advance(std::chrono::seconds(6));

  auto results = storage_->getManySorted("", 10);

  for (const auto& [key, value] : results) {
    EXPECT_NE(key, "short_ttl");
  }

  bool found_persistent = false;
  bool found_long_ttl = false;
  for (const auto& [key, value] : results) {
    if (key == "persistent") found_persistent = true;
    if (key == "long_ttl") found_long_ttl = true;
  }
  EXPECT_TRUE(found_persistent);
  EXPECT_TRUE(found_long_ttl);
}

class KVStoragePerformanceTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std::vector<std::tuple<std::string, std::string, uint32_t>> large_data;
    large_data.reserve(1000);

    for (int i = 0; i < 1000; ++i) {
      large_data.emplace_back("key" + std::to_string(i),
                              "value" + std::to_string(i), 0);
    }

    storage_ = std::make_unique<vk::KVStorage<std::chrono::steady_clock>>(
        std::span(large_data));
  }

  std::unique_ptr<vk::KVStorage<std::chrono::steady_clock>> storage_;
};

TEST_F(KVStoragePerformanceTest, ManyOperations) {
  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < 100; ++i) {
    storage_->get("key" + std::to_string(i));
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  std::cout << "100 get operations took: " << duration.count()
            << " microseconds" << std::endl;
  EXPECT_LT(duration.count(), 10000);
}

TEST_F(KVStoragePerformanceTest, GetManySortedPerformance) {
  auto start = std::chrono::high_resolution_clock::now();

  auto results = storage_->getManySorted("key1", 100);

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  std::cout << "getManySorted(100) took: " << duration.count()
            << " microseconds" << std::endl;
  EXPECT_EQ(results.size(), 100);
  EXPECT_LT(duration.count(), 1000);
}

// Тесты edge cases
TEST(KVStorageEdgeCases, EmptyKeys) {
  vk::KVStorage<std::chrono::steady_clock> storage(
      std::span<std::tuple<std::string, std::string, uint32_t>>{});

  storage.set("", "empty_key_value", 0);
  auto value = storage.get("");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, "empty_key_value");

  EXPECT_TRUE(storage.remove(""));
  EXPECT_FALSE(storage.get("").has_value());
}

TEST(KVStorageEdgeCases, LargeValues) {
  vk::KVStorage<std::chrono::steady_clock> storage(
      std::span<std::tuple<std::string, std::string, uint32_t>>{});

  std::string large_value(10000, 'x');
  storage.set("large", large_value, 0);

  auto retrieved = storage.get("large");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->size(), 10000);
  EXPECT_EQ(*retrieved, large_value);
}
