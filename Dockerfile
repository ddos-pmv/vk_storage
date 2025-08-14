FROM ubuntu:22.04

# Установка зависимостей
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

# Копирование кода
WORKDIR /app
COPY . .

# Сборка
RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Запуск тестов по умолчанию
CMD ["./build/tests/vk_storage_tests"]
