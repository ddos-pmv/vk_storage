#include <gtest/gtest.h>

#include <vk/storage.h>


class KVStorageGossip : public ::testing::Test {
protected:
    void SetUp() override {
        std::vector<std::tuple<std::string, std::string, uint32_t>> initial_data = {
            {"key1", "value1", 0}, {"key2", "value2", 10}, {"key3", "value3", 5}};

        storage_ = std::make_unique<vk::KVStorage<>>();
        storage_->store(std::span(initial_data));
    }

    std::unique_ptr<vk::KVStorage<>> storage_;
};


TEST_F(KVStorageGossip, BasicMerge) {
    auto value = storage_->get("key1");
    ASSERT_EQ(value, "value1");
    storage_->set_if_newer("key1", "value1.1", 0, std::chrono::steady_clock::now());
    value = storage_->get("key1");
    ASSERT_EQ(value, "value1.1");
}


TEST_F(KVStorageGossip, BasicMergeRejected) {
    std::string val11 = "value1.1";
    std::string val12 = "value1.2";
    auto time_point =  std::chrono::steady_clock::now();
    storage_->set_if_newer("key1", val11, 0, std::chrono::steady_clock::now());
    ASSERT_EQ(storage_->get("key1"), val11);


    storage_->set_if_newer("key1", val12, 0, time_point);
    ASSERT_EQ(storage_->get("key1"), val11);
}