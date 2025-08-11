#include <vk/storage.h>

int main() {
  vk::KVStorage<> strg({});

  strg.removeOneExpiredEntry();

  return 0;
}