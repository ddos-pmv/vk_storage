# VK Storage - Key-Value Storage Implementation

VK Internship Test Assignment - высокопроизводительное key-value хранилище с поддержкой TTL.

## Архитектура

### Основные компоненты:
- **Entry** - запись с интрузивными итераторами для O(1) удаления
- **3 индекса**: hash table (ключи), sorted map (ranges), multiset (TTL)
- **Template Clock** - абстракция времени для тестируемости

### Структуры данных:
```cpp
EntryList entries_;              // std::list<Entry> - основное хранилище
KeyIndex key_index_;             // std::unordered_map - O(1) доступ по ключу
SortedIndex sorted_index_;       // std::map - O(log n) sorted ranges
TTLIndex ttl_index_;             // std::multiset - O(log n) TTL управление
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
    std::string key;                     // ~32 bytes (+ key data)
    std::string value;                   // ~32 bytes (+ value data)
    TimePoint expiry_time;               // 8 bytes
    bool has_ttl;                        // 1 byte + 7 padding
    
    // Интрузивные итераторы для O(1) удаления:
    KeyIndexIterator key_index_it;       // ~24 bytes
    SortedIndexIterator sorted_index_it; // ~24 bytes  
    TTLIndexIterator ttl_index_it;       // ~24 bytes
};
// Итого: ~152 bytes per entry + key/value data
```

### Container Overhead:
- **std::list nodes**: ~24 bytes per node (prev, next, data)
- **std::unordered_map buckets**: ~16 bytes per bucket + load factor overhead
- **std::map nodes**: ~40 bytes per node (RB-tree: left, right, parent, color, data)
- **std::multiset nodes**: ~40 bytes per node

### Общий overhead на запись: ~230-250 bytes

## Ключевые особенности реализации

### 1. Интрузивные итераторы
```cpp
// Каждый Entry хранит итераторы на самого себя во всех индексах
KeyIndexIterator key_index_it;     // для O(1) удаления из hash table
SortedIndexIterator sorted_index_it; // для O(1) удаления из sorted map  
TTLIndexIterator ttl_index_it;     // для O(1) удаления из TTL set
```

### 2. Lazy TTL Expiration
- Проверка истечения TTL только при доступе к записи
- `removeOneExpiredEntry()` для активной очистки
- Минимальный overhead на проверку времени


## Сборка

```bash
mkdir build && cd build
cmake ..
make

# Запуск тестов
./tests/vk_storage_tests

# Запуск примера
./examples/example_1
```

## Использование

```cpp
#include <vk/storage.h>

// Создание с начальными данными
std::vector<std::tuple<std::string, std::string, uint32_t>> data = {
    {"key1", "value1", 0},    // без TTL
    {"key2", "value2", 60}    // TTL 60 секунд
};

vk::KVStorage storage(std::span(data));

// Основные операции
storage.set("key3", "value3", 30);          // O(log n)
auto value = storage.get("key1");           // O(1)
bool removed = storage.remove("key2");      // O(1)

// Sorted range queries
auto results = storage.getManySorted("key", 10); // O(log n + k)

// TTL cleanup
auto expired = storage.removeOneExpiredEntry(); // O(1)
```

## Оптимизации для production

1. **Memory Pool** для Entry объектов
2. **Sharding** для многопоточности  
3. **Compressed indices** для memory efficiency
