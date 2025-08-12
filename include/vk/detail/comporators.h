#pragma once
#include <vk/detail/entry.h>

namespace vk::detail {
template <typename Clock>
struct TTLComporator {
  using Entry = Entry<Clock>;
  using EntryInterator =
      typename std::unordered_map<std::string_view,
                                  std::shared_ptr<Entry>>::iterator;
  bool operator()(const EntryInterator& a, const EntryInterator& b) const;
};
}  // namespace vk::detail

#include <vk/impl/comporators.tpp>