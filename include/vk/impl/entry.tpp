
namespace vk::detail {

template <typename Clock>
Entry<Clock>::Entry(std::string k, std::string v, TimePoint exp, bool ttl)
    : key(std::move(k)), value(std::move(v)), expiry_time(exp), has_ttl(ttl) {}

template <typename Clock>
void Entry<Clock>::update_value(std::string new_val) {
  value = std::move(new_val);
}

template <typename Clock>
void Entry<Clock>::update_ttl(TimePoint new_time, bool new_has_ttl) {
  expiry_time = new_time;
  has_ttl = new_has_ttl;
}

template <typename Clock>
bool Entry<Clock>::is_expired(const Clock& clock) const {
  return has_ttl && clock.now() >= expiry_time;
}
}  // namespace vk::detail
