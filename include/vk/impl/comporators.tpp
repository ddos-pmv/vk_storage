namespace vk::detail {

template <typename Clock>
bool TTLComporator<Clock>::operator()(const EntryInterator& a,
                                      const EntryInterator& b) const {
  return a->expire_time < b->expire_time;
}
}  // namespace vk::detail