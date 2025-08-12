#pragma once

#include <chrono>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>

namespace vk::detail {

template <typename Clock>
struct Entry {
  using TimePoint = typename Clock::time_point;
  using EntryList = std::list<Entry>;
  using EntryIterator = typename EntryList::iterator;
  using KeyIndexIterator =
      typename std::unordered_map<std::string_view, EntryIterator>::iterator;
  using SortedIndexIterator =
      typename std::map<std::string_view, EntryIterator>::iterator;
  using TTLIndexIterator = typename std::multiset<EntryIterator>::iterator;

  std::string key;
  std::string value;
  TimePoint expiry_time;
  bool has_ttl = false;

  // Итераторы для O(1) удаления
  KeyIndexIterator key_index_it;
  SortedIndexIterator sorted_index_it;
  TTLIndexIterator ttl_index_it;  // валиден только если has_ttl == true

  Entry(std::string k, std::string v, TimePoint exp, bool ttl);
  void update_value(std::string new_val);
  void update_ttl(TimePoint new_time, bool new_has_ttl);
  bool is_expired(const Clock& clock) const;
};
}  // namespace vk::detail

#include <vk/impl/entry.tpp>