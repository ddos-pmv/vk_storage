#include <vk/storage.h>

#include <iostream>
#include <memory>
#include <memory_resource>
#include <set>
#include <unordered_map>

struct Entry {
  std::string key;
  std::string value;

  bool has_ttl = false;

  std::pmr::unordered_map<int, int> mp;
  // Итераторы для быстрого удаления O(1)
  std::unordered_map<std::string_view, Entry*>::iterator hash_it;
  std::map<std::string_view, Entry*>::iterator sorted_it;
  std::multiset<Entry*>::iterator ttl_it;  // только если has_ttl == true
};

int main() {
  std::string s =
      "1312312312321312321sajkhfalsdjkhflasdjkfhlskjfhlsjdfhasjkhfalskjhf;"
      "dfkjas;fkapijqpwejnr2 3";
  std::string_view sv(s);

  std::unique_ptr<Entry> ptr = std::make_unique<Entry>();
  //   std::shared_ptr<Entry> ptr = std::make_shared<Entry>();
  std::cout << sizeof(std::unique_ptr<Entry>) << " " << sizeof(ptr)
            << std::endl;

  std::cout
      << sizeof(std::unordered_map<std::string_view,
                                   std::shared_ptr<Entry> >::iterator)
      << " "
      << sizeof(std::map<std::string_view, std::shared_ptr<Entry> >::iterator)
      << std::endl;

  std::cout << sizeof(std::string_view) << " " << sizeof(s) << "  "
            << sizeof(sv) << std::endl;

  return 0;
}