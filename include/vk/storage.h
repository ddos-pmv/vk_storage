#pragma once

#include <vk/detail/comporators.h>
#include <vk/detail/entry.h>

#include <chrono>
#include <map>
#include <set>
#include <span>
#include <tuple>
#include <unordered_map>

namespace vk {
template <typename Clock = std::chrono::steady_clock>
class KVStorage {
 public:
  explicit KVStorage(
      std::span<std::tuple<std::string, std::string, uint32_t>> entires,
      Clock clock = Clock());
  ~KVStorage() {}

  void set(std::string key, std::string value, uint32_t ttl);
  bool remove(std::string key);
  std::optional<std::string> get(std::string key);
  std::vector<std::pair<std::string, std::string>> getManySorted(
      std::string_view key, uint32_t count) const;
  std::optional<std::pair<std::string, std::string>> removeOneExpiredEntry();

 private:
  using Entry = detail::Entry<Clock>;
  using EntryIterator =
      std::unordered_map<std::string_view, std::shared_ptr<Entry>>::iterator;
  using TTLComporator = typename detail::TTLComporator<Clock>;

  Clock clock_;
  std::unordered_map<std::string_view, std::shared_ptr<Entry>> key_index_;
  std::map<std::string_view, EntryIterator> sorted_index_;
  std::multiset<EntryIterator, TTLComporator> ttl_sorted_index_;
};
}  // namespace vk

#include <vk/impl/storage.tpp>