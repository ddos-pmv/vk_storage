#include <gtest/gtest.h>
#include <vk/storage.h>

#include <chrono>
#include <memory>
#include <thread>

#include "mock_clock.h"

class KVStorageTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std::vector<std::tuple<std::string, std::string, uint32_t>> initial_data = {
        {"key1", "value1", 0}, {"key2", "value2", 10}, {"key3", "value3", 5}};
    storage_ = std::make_unique<vk::KVStorage<std::chrono::steady_clock>>(
        std::span<std::tuple<std::string, std::string, uint32_t>>(
            initial_data));
  }

  std::unique_ptr<vk::KVStorage<std::chrono::steady_clock>> storage_;
};

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
  storage_->set("key1", "value_with_ttl", 3600);
  auto value = storage_->get("key1");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, "value_with_ttl");
}

TEST_F(KVStorageTest, Remove) {
  EXPECT_TRUE(storage_->remove("key1"));
  EXPECT_FALSE(storage_->get("key1").has_value());
  EXPECT_FALSE(storage_->remove("key1"));
}

TEST_F(KVStorageTest, RemoveNonExistent) {
  EXPECT_FALSE(storage_->remove("non_existent_key"));
}

TEST_F(KVStorageTest, GetManySorted) {
  storage_->set("a_key", "a_value", 0);
  storage_->set("z_key", "z_value", 0);

  auto results = storage_->getManySorted("", 10);

  EXPECT_GE(results.size(), 3);
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

class KVStorageTTLTest : public ::testing::Test {
 protected:
  void SetUp() override {
    clock_.setup_static();
    std::vector<std::tuple<std::string, std::string, uint32_t>> initial_data = {
        {"persistent", "value", 0},
        {"short_ttl", "value", 5},
        {"long_ttl", "value", 3600}};
    storage_ = std::make_unique<vk::KVStorage<MockClock>>(
        std::span<std::tuple<std::string, std::string, uint32_t>>(initial_data),
        clock_);
  }

  MockClock clock_;
  std::unique_ptr<vk::KVStorage<MockClock>> storage_;
};

TEST_F(KVStorageTTLTest, TTLExpiration) {
  EXPECT_TRUE(storage_->get("short_ttl").has_value());
  EXPECT_TRUE(storage_->get("long_ttl").has_value());
  EXPECT_TRUE(storage_->get("persistent").has_value());

  clock_.advance(std::chrono::seconds(6));

  EXPECT_FALSE(storage_->get("short_ttl").has_value());
  EXPECT_TRUE(storage_->get("long_ttl").has_value());
  EXPECT_TRUE(storage_->get("persistent").has_value());
}

TEST_F(KVStorageTTLTest, RemoveExpiredEntry) {
  clock_.advance(std::chrono::seconds(6));

  auto expired = storage_->removeOneExpiredEntry();
  ASSERT_TRUE(expired.has_value());
  EXPECT_EQ(expired->first, "short_ttl");
  EXPECT_EQ(expired->second, "value");

  EXPECT_TRUE(storage_->get("persistent").has_value());
}

TEST_F(KVStorageTTLTest, UpdateTTL) {
  storage_->set("short_ttl", "new_value", 3600);

  clock_.advance(std::chrono::seconds(6));

  auto value = storage_->get("short_ttl");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, "new_value");
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
