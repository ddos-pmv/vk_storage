#pragma once

#include <boost/intrusive/list_hook.hpp>
#include <boost/intrusive/set_hook.hpp>
#include <boost/intrusive/unordered_set_hook.hpp>
#include <string>

namespace vk::detail {

template <typename Clock>
struct Entry {
  using TimePoint = typename Clock::time_point;

  std::string key;
  std::string value;
  TimePoint expiry_time;
  bool has_ttl = false;

  // Хуки
  boost::intrusive::unordered_set_member_hook<> hash_hook_;
  boost::intrusive::set_member_hook<> set_hook_;
  boost::intrusive::set_member_hook<> ttl_hook_;
  boost::intrusive::list_member_hook<> memory_hook_;

  Entry(std::string k, std::string v, TimePoint exp, bool ttl);

  void update_value(std::string new_val);
  void update_ttl(TimePoint new_time, bool new_has_ttl);
  bool is_expired(const Clock& clock) const;
};

}  // namespace vk::detail

#include <vk/impl/entry.tpp>