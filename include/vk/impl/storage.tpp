namespace vk {

template <typename Clock>
KVStorage<Clock>::KVStorage(
    std::span<std::tuple<std::string, std::string, uint32_t>> entries,
    Clock clock)
    : clock_(clock) {
  for (auto& [key, value, ttl] : entries) {
    set(std::move(key), std::move(value), ttl);
  }
}

template <typename Clock>
void KVStorage<Clock>::set(std::string key, std::string value, uint32_t ttl) {}

}  // namespace vk