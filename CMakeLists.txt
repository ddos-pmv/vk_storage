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
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# ===== ОСНОВНАЯ БИБЛИОТЕКА =====
file(GLOB_RECURSE VK_STORAGE_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE VK_STORAGE_HEADERS 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

add_library(vk_storage INTERFACE)

target_include_directories(vk_storage INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_sources(vk_storage INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/vk/storage.h>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/vk/detail/entry.h>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/vk/detail/comparators.h>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/vk/impl/entry.tpp>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/vk/impl/comparators.tpp>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/vk/impl/storage.tpp>
)

# Устанавливаем свойства библиотеки
# set_target_properties(vk_storage PROPERTIES
#     VERSION 1.0.0
#     SOVERSION 1
#     PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/vk/storage.h"
# )

# ===== ПРИМЕРЫ =====
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ===== ТЕСТЫ =====
# if(BUILD_TESTS)
#     add_subdirectory(tests)
# endif()

# ===== УСТАНОВКА =====
# include(GNUInstallDirs)

# install(TARGETS vk_storage
#     EXPORT vk_storageTargets
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vk
# )

# # Устанавливаем все публичные заголовки
# install(DIRECTORY include/
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#     FILES_MATCHING PATTERN "*.h"
# )

# install(EXPORT vk_storageTargets
#     FILE vk_storageTargets.cmake
#     NAMESPACE vk::
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vk_storage
# )
