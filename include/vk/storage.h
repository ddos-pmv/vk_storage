#pragma once

#include <vk/detail/comparators.h>
#include <vk/detail/entry.h>

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
  using Entry = detail::Entry<Clock>;
  using EntryList = std::list<Entry>;
  using EntryIterator = typename EntryList::iterator;
  using TTLComparator = detail::TTLComparator<Clock>;

  Clock clock_;
  EntryList entries_;
  std::unordered_map<std::string_view, EntryIterator> key_index_;
  std::map<std::string_view, EntryIterator> sorted_index_;
  std::multiset<EntryIterator, TTLComparator> ttl_index_;
};
}  // namespace vk

#include <vk/impl/storage.tpp>