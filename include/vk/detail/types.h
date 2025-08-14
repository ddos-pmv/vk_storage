#pragma once

#include <boost/functional/hash.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/set.hpp>
#include <boost/intrusive/unordered_set.hpp>
#include <chrono>
#include <string>
#include <string_view>

#include "entry.h"

namespace vk::detail {

template <typename Clock>
struct Types {
  using TimePoint = typename Clock::time_point;
  using EntryType = Entry<Clock>;

  struct KeyHash {
    std::size_t operator()(const EntryType& entry) const {
      return boost::hash<std::string>{}(entry.key);
    }
  };

  struct KeyEqual {
    bool operator()(const EntryType& a, const EntryType& b) const {
      return a.key == b.key;
    }
  };

  struct KeyCompare {
    bool operator()(const EntryType& a, const EntryType& b) const {
      return a.key < b.key;
    }
  };

  struct TTLCompare {
    bool operator()(const EntryType& a, const EntryType& b) const {
      if (!a.has_ttl && !b.has_ttl) return false;
      if (!a.has_ttl) return false;
      if (!b.has_ttl) return true;

      if (a.expiry_time != b.expiry_time) {
        return a.expiry_time < b.expiry_time;
      }

      return &a < &b;
    }
  };

  // Member hook options
  using HashMemberOption = boost::intrusive::member_hook<
      EntryType, boost::intrusive::unordered_set_member_hook<>,
      &EntryType::hash_hook_>;
  using SortedMemberOption = boost::intrusive::member_hook<
      EntryType, boost::intrusive::set_member_hook<>, &EntryType::set_hook_>;
  using TTLMemberOption = boost::intrusive::member_hook<
      EntryType, boost::intrusive::set_member_hook<>, &EntryType::ttl_hook_>;
  using ListMemberOption =
      boost::intrusive::member_hook<EntryType,
                                    boost::intrusive::list_member_hook<>,
                                    &EntryType::memory_hook_>;

  using MemoryList = boost::intrusive::list<EntryType, ListMemberOption>;
  using KeyIndex =
      boost::intrusive::unordered_set<EntryType, HashMemberOption,
                                      boost::intrusive::hash<KeyHash>,
                                      boost::intrusive::equal<KeyEqual>>;
  using SortedIndex =
      boost::intrusive::set<EntryType, SortedMemberOption,
                            boost::intrusive::compare<KeyCompare>>;
  using TTLIndex =
      boost::intrusive::multiset<EntryType, TTLMemberOption,
                                 boost::intrusive::compare<TTLCompare>>;
  // Bucket types
  using Bucket = KeyIndex::bucket_type;
  using BucketTraits = KeyIndex::bucket_traits;
};

}  // namespace vk::detail