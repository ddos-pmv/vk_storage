#pragma once

#include <vk/detail/types.h>

namespace vk::detail {

template <typename Clock>
struct Entry {
  using Types = Types<Clock>;
  using TimePoint = typename Types::TimePoint;
  using KeyIndexIterator = typename Types::KeyIndexIterator;
  using SortedIndexIterator = typename Types::SortedIndexIterator;
  using TTLIndexIterator = typename Types::TTLIndexIterator;

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