#pragma once

#include <chrono>
#include <map>
#include <span>
#include <tuple>
#include <unordered_map>

namespace vk {
template <typename Clock = std::chrono::steady_clock>
class KVStorage {
 public:
  explicit KVStorage(
      std::span<std::tuple<std::string, std::string, uint32_t>> entires,
      Clock clock = Clock()) {}

  ~KVStorage() {}

  void set(std::string key, std::string value, uint32_t ttl) { return; }
  bool remove(std::string key) { return false; }
  std::optional<std::string> get(std::string key) { return std::nullopt; }

  std::vector<std::pair<std::string, std::string>> getManySorted(
      std::string_view key, uint32_t count) const {
    return {};
  }

  std::optional<std::pair<std::string, std::string>> removeOneExpiredEntry() {
    return std::nullopt;
  }

 private:
};
}  // namespace vk