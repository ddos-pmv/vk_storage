#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace vk::detail {

template <typename Clock>
struct Entry {
  using TimePoint = typename Clock::time_point;
  using EntryInterator =
      typename std::unordered_map<std::string_view,
                                  std::shared_ptr<Entry>>::iterator;

  std::string key;
  std::string value;
  TimePoint expire_time;
};
}  // namespace vk::detail