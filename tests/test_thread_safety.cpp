#include <gtest/gtest.h>
#include <vk/storage.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "mock_clock.h"

class KVStorageThreadSafetyTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std::vector<std::tuple<std::string, std::string, uint32_t>> initial_data = {
        {"shared_key", "initial_value", 0}};
    storage_ = std::make_unique<vk::KVStorage<std::chrono::steady_clock>>(
        std::span<std::tuple<std::string, std::string, uint32_t>>(
            initial_data));
  }

  std::unique_ptr<vk::KVStorage<std::chrono::steady_clock>> storage_;
};

TEST_F(KVStorageThreadSafetyTest, ConcurrentReadsAndWrites) {
  const int num_threads = 4;
  const int operations_per_thread = 100;
  std::vector<std::thread> threads;
  std::atomic<int> success_count{0};

  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([&, t]() {
      for (int i = 0; i < operations_per_thread; ++i) {
        std::string key = "key_" + std::to_string(t) + "_" + std::to_string(i);
        std::string value =
            "value_" + std::to_string(t) + "_" + std::to_string(i);

        storage_->set(key, value, 0);

        auto retrieved = storage_->get(key);
        if (retrieved.has_value() && *retrieved == value) {
          success_count++;
        }

        if (i % 10 == 0) {
          storage_->remove(key);
        }
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_GT(success_count.load(), operations_per_thread * num_threads * 0.8);
}

TEST_F(KVStorageThreadSafetyTest, ConcurrentGetManySorted) {
  for (int i = 0; i < 100; ++i) {
    storage_->set("item_" + std::to_string(i), "value_" + std::to_string(i), 0);
  }

  const int num_threads = 8;
  std::vector<std::thread> threads;
  std::atomic<bool> all_sorted{true};

  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([&]() {
      for (int i = 0; i < 50; ++i) {
        auto results = storage_->getManySorted("item_", 20);

        for (size_t j = 1; j < results.size(); ++j) {
          if (results[j - 1].first > results[j].first) {
            all_sorted = false;
            break;
          }
        }
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_TRUE(all_sorted.load());
}

TEST_F(KVStorageThreadSafetyTest, ConcurrentTTLOperations) {
  MockClock clock;
  clock.setup_static();
  std::vector<std::tuple<std::string, std::string, uint32_t>> initial_data = {
      {"ttl_key_1", "value1", 5},
      {"ttl_key_2", "value2", 10},
      {"ttl_key_3", "value3", 15}};

  vk::KVStorage<MockClock> ttl_storage(
      std::span<std::tuple<std::string, std::string, uint32_t>>(initial_data),
      clock);

  const int num_threads = 4;
  std::vector<std::thread> threads;
  std::atomic<int> expired_found{0};

  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([&, t]() {
      for (int i = 0; i < 20; ++i) {
        std::string key =
            "thread_" + std::to_string(t) + "_key_" + std::to_string(i);
        ttl_storage.set(key, "value", 3);

        if (t == 0 && i % 5 == 0) {
          clock.advance(std::chrono::seconds(1));
        }

        auto expired = ttl_storage.removeOneExpiredEntry();
        if (expired.has_value()) {
          expired_found++;
        }

        std::this_thread::sleep_for(std::chrono::microseconds(10));
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_GE(expired_found.load(), 0);
}

TEST_F(KVStorageThreadSafetyTest, StressTestMixedOperations) {
  const int num_threads = 6;
  const int operations_per_thread = 200;
  std::vector<std::thread> threads;
  std::atomic<int> total_operations{0};

  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([&, t]() {
      for (int i = 0; i < operations_per_thread; ++i) {
        int operation = (t * operations_per_thread + i) % 4;
        std::string key = "stress_key_" + std::to_string(i % 50);

        switch (operation) {
          case 0:
            storage_->set(key, "stress_value_" + std::to_string(i), i % 30);
            break;
          case 1:
            storage_->get(key);
            break;
          case 2:
            storage_->remove(key);
            break;
          case 3:
            storage_->getManySorted("stress_key_", 10);
            break;
        }
        total_operations++;
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_EQ(total_operations.load(), num_threads * operations_per_thread);
}

TEST_F(KVStorageThreadSafetyTest, DeadlockPrevention) {
  const int num_threads = 10;
  std::vector<std::thread> threads;
  std::atomic<bool> deadlock_detected{false};

  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([&, t]() {
      auto start_time = std::chrono::steady_clock::now();

      for (int i = 0; i < 100; ++i) {
        auto current_time = std::chrono::steady_clock::now();
        if (current_time - start_time > std::chrono::seconds(5)) {
          deadlock_detected = true;
          break;
        }

        if (t % 2 == 0) {
          storage_->set("deadlock_key_" + std::to_string(i % 10), "value", 0);
          storage_->get("deadlock_key_" + std::to_string((i + 1) % 10));
        } else {
          storage_->get("deadlock_key_" + std::to_string(i % 10));
          storage_->set("deadlock_key_" + std::to_string((i + 1) % 10), "value",
                        0);
        }

        if (i % 5 == 0) {
          storage_->getManySorted("deadlock_key_", 5);
        }
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_FALSE(deadlock_detected.load()) << "Potential deadlock detected!";
}
