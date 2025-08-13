cmake_minimum_required(VERSION 3.20)
project(KVStorage CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Опции сборки
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# ===== Boost полностью =====
include(FetchContent)

FetchContent_Declare(
  Boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.83.0/boost-1.83.0.tar.xz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(BOOST_ENABLE_CMAKE ON)
set(BOOST_INCLUDE_LIBRARIES "intrusive")

FetchContent_MakeAvailable(Boost)

# ===== ОСНОВНАЯ БИБЛИОТЕКА =====
add_library(vk_storage INTERFACE)

target_include_directories(vk_storage INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_sources(vk_storage INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/vk/storage.h>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/vk/detail/entry.h>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/vk/detail/types.h>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/vk/impl/entry.tpp>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/vk/impl/storage.tpp>
)

target_link_libraries(vk_storage INTERFACE Boost::intrusive)

# ===== ПРИМЕРЫ =====
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ===== ТЕСТЫ =====
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()