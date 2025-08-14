#pragma once
#include <vk/detail/entry.h>
#include <vk/detail/types.h>

#include <array>
#include <chrono>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>

namespace vk {

template <typename Clock = std::chrono::steady_clock>
class KVStorage {
 public:
  explicit KVStorage(
      std::span<std::tuple<std::string, std::string, uint32_t>> entries,
      Clock clock = Clock());
  ~KVStorage();

  void set(std::string key, std::string value, uint32_t ttl);
  bool remove(std::string_view key);
  std::optional<std::string> get(std::string_view key) const;
  std::vector<std::pair<std::string, std::string>> getManySorted(
      std::string_view key, uint32_t count) const;
  std::optional<std::pair<std::string, std::string>> removeOneExpiredEntry();

 private:
  using Types = detail::Types<Clock>;
  using Entry = typename Types::EntryType;
  using KeyIndex = typename Types::KeyIndex;
  using SortedIndex = typename Types::SortedIndex;
  using TTLIndex = typename Types::TTLIndex;
  using Bucket = typename Types::Bucket;
  using BucketTraits = typename Types::BucketTraits;
  using MemoryList = typename Types::MemoryList;

  Clock clock_;

  // Buckets для hash table
  static constexpr size_t BUCKET_COUNT = 1024;
  std::array<Bucket, BUCKET_COUNT> buckets_;

  // Intrusive containers
  KeyIndex key_index_;
  SortedIndex sorted_index_;
  TTLIndex ttl_index_;
  MemoryList memory_list_;

  Entry* create_entry(std::string key, std::string value,
                      typename Clock::time_point expiry, bool has_ttl);
  void destroy_entry(Entry* entry);
};

}  // namespace vk

#include <vk/impl/storage.tpp>