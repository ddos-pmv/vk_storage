#pragma once

#include <vk/detail/entry.h>
#include <vk/detail/types.h>

#include <chrono>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace vk {
template <typename Clock = std::chrono::steady_clock>
class KVStorage {
 public:
  explicit KVStorage(
      std::span<std::tuple<std::string, std::string, uint32_t>> entries,
      Clock clock = Clock());
  ~KVStorage() = default;

  void set(std::string key, std::string value, uint32_t ttl);
  bool remove(std::string_view key);
  std::optional<std::string> get(std::string_view key) const;
  std::vector<std::pair<std::string, std::string>> getManySorted(
      std::string_view key, uint32_t count) const;
  std::optional<std::pair<std::string, std::string>> removeOneExpiredEntry();

 private:
  using Types = detail::Types<Clock>;
  using Entry = typename Types::EntryType;
  using EntryList = typename Types::EntryList;
  using EntryIterator = typename Types::EntryIterator;
  using KeyIndex = typename Types::KeyIndex;
  using SortedIndex = typename Types::SortedIndex;
  using TTLIndex = typename Types::TTLIndex;

  Clock clock_;
  EntryList entries_;
  KeyIndex key_index_;
  SortedIndex sorted_index_;
  TTLIndex ttl_index_;
};
}  // namespace vk

#include <vk/impl/storage.tpp>