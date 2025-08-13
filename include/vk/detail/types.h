#pragma once

#include <boost/intrusive/list.hpp>
#include <boost/intrusive/set.hpp>
#include <boost/intrusive/unordered_set.hpp>
#include <chrono>
#include <list>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

namespace vk::detail {

// Forward declaration
template <typename Clock>
struct Entry;

template <typename Clock>
struct Types {
  using TimePoint = typename Clock::time_point;
  using EntryType = Entry<Clock>;
  using EntryList = std::list<EntryType>;
  using EntryIterator = typename EntryList::iterator;
  using KeyIndexIterator =
      typename std::unordered_map<std::string_view, EntryIterator>::iterator;
  using SortedIndexIterator =
      typename std::map<std::string_view, EntryIterator>::iterator;

  // Встроенный компаратор - решает все проблемы
  struct TTLComparator {
    bool operator()(const EntryIterator& a, const EntryIterator& b) const {
      // Записи без TTL идут в конец
      if (!a->has_ttl && !b->has_ttl) return false;
      if (!a->has_ttl) return false;
      if (!b->has_ttl) return true;

      // Сортируем по времени истечения
      if (a->expiry_time != b->expiry_time) {
        return a->expiry_time < b->expiry_time;
      }

      // Для стабильной сортировки используем адрес
      return &(*a) < &(*b);
    }
  };

  using TTLIndexIterator =
      typename std::multiset<EntryIterator, TTLComparator>::iterator;

  // Контейнеры
  using KeyIndex = std::unordered_map<std::string_view, EntryIterator>;
  using SortedIndex = std::map<std::string_view, EntryIterator>;
  using TTLIndex = std::multiset<EntryIterator, TTLComparator>;
};

}  // namespace vk::detail