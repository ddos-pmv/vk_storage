namespace vk::detail {

template <typename Clock>
bool TTLComparator<Clock>::operator()(const EntryIterator& a,
                                      const EntryIterator& b) const {
  if (!a->has_ttl && !b->has_ttl) return false;
  if (!a->has_ttl) return false;
  if (!b->has_ttl) return true;

  if (a->expiry_time != b->expiry_time) {
    return a->expiry_time < b->expiry_time;
  }

  return &(*a) < &(*b);
}
}  // namespace vk::detail