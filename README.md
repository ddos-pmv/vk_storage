# VK Storage - High-Performance Key-Value Storage

Высокопроизводительное key-value хранилище с поддержкой TTL и O(1) удалением.

## Архитектура

## Важно !!!
***Т.к. в задании не было запрета на использование сторнних библиотек, для реализации была использована Boost.Intrusive. Если запрет все же был, то прошу оценить предыдущую реализацию[(ссылка на комит)](https://github.com/ddos-pmv/vk_storage/tree/2ba6ed0ea94cfa0a6e505d22fe1ee3fa85a7dd58), без испльзование сторнних библиотек.***


### Основные компоненты:
- **Entry** - запись с интрузивными хуками для O(1) операций
- **4 интрузивных индекса**: hash table (ключи), sorted set (ranges), TTL set, memory list
- **Template Clock** - абстракция времени для тестируемости

### Интрузивные структуры данных (Boost.Intrusive):
```cpp
MemoryList memory_list_;         // boost::intrusive::list - O(1) create/destroy entry
KeyIndex key_index_;             // boost::intrusive::unordered_set - O(1) доступ по ключу  
SortedIndex sorted_index_;       // boost::intrusive::set - O(log n) sorted ranges
TTLIndex ttl_index_;             // boost::intrusive::set - O(log n) TTL управление
```

## ⚡ Сложность операций

| Операция | Временная сложность | Пространственная сложность |
|----------|-------------------|---------------------------|
| `get(key)` | **O(1)** | O(1) |
| `set(key, value, ttl)` | **O(log n)** | O(1) |
| `remove(key)` | **O(1)** | O(1) |
| `getManySorted(key, count)` | **O(log n + k)** | O(k) |
| `removeOneExpiredEntry()` | **O(1)** | O(1) |

*где n - количество записей, k - количество возвращаемых элементов*

## 💾 Memory Overhead

### Per Entry:
```cpp
struct Entry {
    std::string key;
    std::string value;
    TimePoint expiry_time;                                      // 8 bytes
    bool has_ttl;                                              // 1 byte + 7 padding
    
    // Intrusive hooks - только указатели на соседей в контейнерах:
    boost::intrusive::unordered_set_member_hook<> hash_hook_;   // ~16 bytes
    boost::intrusive::set_member_hook<> sorted_hook_;          // ~24 bytes  
    boost::intrusive::set_member_hook<> ttl_hook_;             // ~24 bytes
    boost::intrusive::list_member_hook<> memory_hook_;         // ~16 bytes
};
```

**Ключевое преимущество**: Boost.Intrusive контейнеры НЕ создают дополнительных узлов!
Все указатели хранятся прямо в Entry объекте.

### Итого: ~100 bytes overhead per Entry (без учета key/value)

## Ключевые особенности реализации

### 1. Полностью интрузивная архитектура
- Каждый Entry содержит хуки для всех 4 контейнеров
- **Никаких дополнительных аллокаций** - все указатели в самом объекте
- O(1) удаление из всех индексов через `iterator_to()`

### 2. Memory Management через intrusive::list
```cpp
// O(1) создание и уничтожение записей
Entry* create_entry(const std::string& key, const std::string& value) {
    auto* entry = new Entry{key, value, ...};
    memory_list_.push_back(*entry);  // O(1) - просто добавить в список
    return entry;
}

void destroy_entry(Entry* entry) {
    memory_list_.erase(memory_list_.iterator_to(*entry));  // O(1)
    delete entry;
}
```

### 3. Lazy TTL Expiration
- Проверка истечения TTL только при доступе к записи
- `removeOneExpiredEntry()` для активной очистки через TTL индекс
- Минимальный overhead на проверку времени


## Требования

- **C++20** (для std::span, string_view, concepts)
- **CMake 3.16+**
- **Boost.Intrusive** (подтягивается автоматически через FetchContent)
- **GTest** (для тестов, подтягивается автоматически)

## Сборка и запуск

```bash
# Клонирование
git clone <https://github.com/ddos-pmv/vk_storage>
cd vk_storage

# Сборка
mkdir build && cd build
cmake ..
make -j$(nproc)

# Запуск тестов
./tests/vk_storage_tests

# Запуск примера
./examples/example_1
```

### Сборка и запуск в Docker
```bash
# Сборка образа
docker build -t vk-storage .

# Интерактивный режим для экспериментов
docker run --rm -it vk-storage bash
```

## Использование

```cpp
#include <vk/storage.h>

// Создание с начальными данными
std::vector<std::tuple<std::string, std::string, uint32_t>> initial_data = {
    {"key1", "value1", 0},    // без TTL
    {"key2", "value2", 60}    // TTL 60 секунд
};

vk::KVStorage storage(std::span(initial_data));

// Основные операции
storage.set("key3", "value3", 30);          // O(log n) - добавить в sorted и TTL индексы
auto value = storage.get("key1");           // O(1) - поиск в hash индексе
bool removed = storage.remove("key2");      // O(1) - удаление из всех индексов

// Sorted range queries  
auto results = storage.getManySorted("key", 10); // O(log n + k) - поиск в sorted индексе

// TTL cleanup
auto expired = storage.removeOneExpiredEntry(); // O(1) - удаление самой старой записи
```

## Возможные улучшения для production

1. **Custom Allocator**: заменить new/delete на pool allocator
2. **Sharding**: разделить на несколько independent shards для многопоточности
