namespace vk {

template <typename Clock>
KVStorage<Clock>::KVStorage(
    std::span<std::tuple<std::string, std::string, uint32_t>> entries,
    Clock clock)
    : clock_(clock) {
  for (const auto& [key, value, ttl] : entries) {
    set(key, value, ttl);
  }
}

template <typename Clock>
void KVStorage<Clock>::set(std::string key, std::string value, uint32_t ttl) {
  auto existing = key_index_.find(key);
  if (existing != key_index_.end()) {
    // Обновляем существующую запись
    EntryIterator entry_it = existing->second;

    // Удаляем из TTL индекса используя сохраненный итератор O(1)
    if (entry_it->has_ttl) {
      ttl_index_.erase(entry_it->ttl_index_it);
    }

    // Обновляем значение
    entry_it->update_value(std::move(value));

    // Обновляем TTL
    if (ttl > 0) {
      auto expiry = clock_.now() + std::chrono::seconds(ttl);
      entry_it->update_ttl(expiry, true);
      entry_it->ttl_index_it = ttl_index_.insert(entry_it);
    } else {
      entry_it->update_ttl(typename Clock::time_point{}, false);
    }
  } else {
    // Создаем новую запись
    auto expiry = (ttl > 0) ? clock_.now() + std::chrono::seconds(ttl)
                            : typename Clock::time_point{};
    bool has_ttl_flag = ttl > 0;

    entries_.emplace_back(key, std::move(value), expiry, has_ttl_flag);
    EntryIterator entry_it = std::prev(entries_.end());

    // Добавляем в индексы и сохраняем итераторы
    entry_it->key_index_it =
        key_index_.emplace(std::string_view(entry_it->key), entry_it).first;
    entry_it->sorted_index_it =
        sorted_index_.emplace(std::string_view(entry_it->key), entry_it).first;

    if (has_ttl_flag) {
      entry_it->ttl_index_it = ttl_index_.insert(entry_it);
    }
  }
}

template <typename Clock>
bool KVStorage<Clock>::remove(std::string_view key) {
  auto it = key_index_.find(key);
  if (it == key_index_.end()) {
    return false;
  }

  EntryIterator entry_it = it->second;

  // Удаляем из всех индексов за O(1) используя сохраненные итераторы!
  key_index_.erase(entry_it->key_index_it);
  sorted_index_.erase(entry_it->sorted_index_it);

  if (entry_it->has_ttl) {
    ttl_index_.erase(entry_it->ttl_index_it);
  }

  // Удаляем из основного контейнера O(1)
  entries_.erase(entry_it);

  return true;
}

template <typename Clock>
std::optional<std::string> KVStorage<Clock>::get(std::string_view key) const {
  auto it = key_index_.find(key);
  if (it == key_index_.end()) {
    return std::nullopt;
  }

  EntryIterator entry_it = it->second;

  // Проверяем TTL
  if (entry_it->is_expired(clock_)) {
    return std::nullopt;
  }

  return entry_it->value;
}

template <typename Clock>
std::vector<std::pair<std::string, std::string>>
KVStorage<Clock>::getManySorted(std::string_view key, uint32_t count) const {
  std::vector<std::pair<std::string, std::string>> result;
  result.reserve(count);

  auto it = sorted_index_.lower_bound(key);

  while (it != sorted_index_.end() && result.size() < count) {
    EntryIterator entry_it = it->second;

    // Пропускаем истекшие записи
    if (!entry_it->is_expired(clock_)) {
      result.emplace_back(entry_it->key, entry_it->value);
    }

    ++it;
  }

  return result;
}

template <typename Clock>
std::optional<std::pair<std::string, std::string>>
KVStorage<Clock>::removeOneExpiredEntry() {
  auto it = ttl_index_.begin();
  if (it != ttl_index_.end()) {
    EntryIterator entry_it = *it;
    if (entry_it->is_expired(clock_)) {
      // Нашли истекшую запись
      std::pair<std::string, std::string> result{entry_it->key,
                                                 entry_it->value};

      // Удаляем за O(1) используя итераторы!
      key_index_.erase(entry_it->key_index_it);
      sorted_index_.erase(entry_it->sorted_index_it);
      ttl_index_.erase(it);  // уже имеем итератор

      entries_.erase(entry_it);

      return result;
    }
  }

  return std::nullopt;
}

}  // namespace vk