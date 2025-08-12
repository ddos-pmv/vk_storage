#pragma once
#include <vk/detail/entry.h>

namespace vk::detail {
template <typename Clock>
struct TTLComparator {
  using Entry = Entry<Clock>;
  using EntryIterator = typename std::list<Entry>::iterator;

  bool operator()(const EntryIterator& a, const EntryIterator& b) const;
};
}  // namespace vk::detail

#include <vk/impl/comparators.tpp>