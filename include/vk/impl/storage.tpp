#include <algorithm>

namespace vk {

template <typename Clock>
KVStorage<Clock>::KVStorage(
    std::span<std::tuple<std::string, std::string, uint32_t>> entries,
    Clock clock)
    : clock_(clock),
      key_index_(BucketTraits(buckets_.data(), buckets_.size())) {
  for (const auto& [key, value, ttl] : entries) {
    set(key, value, ttl);
  }
}

template <typename Clock>
KVStorage<Clock>::~KVStorage() {
  key_index_.clear();
  sorted_index_.clear();
  ttl_index_.clear();

  while (!memory_list_.empty()) {
    Entry* entry = &memory_list_.front();
    memory_list_.pop_front();
    delete entry;
  }
}

template <typename Clock>
void KVStorage<Clock>::set(std::string key, std::string value, uint32_t ttl) {
  Entry search_entry(key, "", typename Clock::time_point{}, false);
  auto existing = key_index_.find(search_entry);

  if (existing != key_index_.end()) {
    Entry& entry = *existing;

    if (entry.has_ttl) {
      ttl_index_.erase(ttl_index_.iterator_to(entry));
    }

    entry.update_value(std::move(value));

    if (ttl > 0) {
      auto expiry = clock_.now() + std::chrono::seconds(ttl);
      entry.update_ttl(expiry, true);
      ttl_index_.insert(entry);
    } else {
      entry.update_ttl(typename Clock::time_point{}, false);
    }
  } else {
    auto expiry = (ttl > 0) ? clock_.now() + std::chrono::seconds(ttl)
                            : typename Clock::time_point{};
    bool has_ttl_flag = ttl > 0;

    Entry* entry =
        create_entry(std::move(key), std::move(value), expiry, has_ttl_flag);

    key_index_.insert(*entry);
    sorted_index_.insert(*entry);
    memory_list_.push_back(*entry);

    if (has_ttl_flag) {
      ttl_index_.insert(*entry);
    }
  }
}

template <typename Clock>
bool KVStorage<Clock>::remove(std::string_view key) {
  Entry search_entry(std::string(key), "", typename Clock::time_point{}, false);
  auto it = key_index_.find(search_entry);

  if (it == key_index_.end()) {
    return false;
  }

  Entry& entry = *it;

  key_index_.erase(it);
  sorted_index_.erase(sorted_index_.iterator_to(entry));

  if (entry.has_ttl) {
    ttl_index_.erase(ttl_index_.iterator_to(entry));
  }

  memory_list_.erase(memory_list_.iterator_to(entry));
  destroy_entry(&entry);

  return true;
}

template <typename Clock>
std::optional<std::string> KVStorage<Clock>::get(std::string_view key) const {
  Entry search_entry(std::string(key), "", typename Clock::time_point{}, false);
  auto it = key_index_.find(search_entry);

  if (it == key_index_.end()) {
    return std::nullopt;
  }

  const Entry& entry = *it;

  if (entry.is_expired(clock_)) {
    return std::nullopt;
  }

  return entry.value;
}

template <typename Clock>
std::vector<std::pair<std::string, std::string>>
KVStorage<Clock>::getManySorted(std::string_view key, uint32_t count) const {
  std::vector<std::pair<std::string, std::string>> result;
  result.reserve(count);

  Entry search_entry(std::string(key), "", typename Clock::time_point{}, false);
  auto it = sorted_index_.lower_bound(search_entry);

  while (it != sorted_index_.end() && result.size() < count) {
    const Entry& entry = *it;

    if (!entry.is_expired(clock_)) {
      result.emplace_back(entry.key, entry.value);
    }

    ++it;
  }

  return result;
}

template <typename Clock>
std::optional<std::pair<std::string, std::string>>
KVStorage<Clock>::removeOneExpiredEntry() {
  if (ttl_index_.empty()) {
    return std::nullopt;
  }

  auto it = ttl_index_.begin();
  Entry& entry = *it;

  if (!entry.is_expired(clock_)) {
    return std::nullopt;
  }

  std::pair<std::string, std::string> result{entry.key, entry.value};

  // Удаляем из всех индексов
  key_index_.erase(key_index_.iterator_to(entry));
  sorted_index_.erase(sorted_index_.iterator_to(entry));
  ttl_index_.erase(it);
  memory_list_.erase(memory_list_.iterator_to(entry));

  // Освобождаем память
  destroy_entry(&entry);

  return result;
}

template <typename Clock>
typename KVStorage<Clock>::Entry* KVStorage<Clock>::create_entry(
    std::string key, std::string value, typename Clock::time_point expiry,
    bool has_ttl) {
  // Простое выделение памяти - никаких проблем с boost::pool!
  return new Entry(std::move(key), std::move(value), expiry, has_ttl);
}

template <typename Clock>
void KVStorage<Clock>::destroy_entry(Entry* entry) {
  delete entry;
}

}  // namespace vk