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
option(INSTALL_VK_STORAGE "Install VK Storage library" ON)

# ===== ОСНОВНАЯ БИБЛИОТЕКА =====
# file(GLOB_RECURSE VK_STORAGE_SOURCES 
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
# )

add_library(vk_storage INTERFACE)
target_include_directories(vk_storage INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ===== ПРИМЕРЫ =====
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ===== ТЕСТЫ =====
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# ===== УСТАНОВКА =====
# if(INSTALL_VK_STORAGE)
#     include(GNUInstallDirs)
    
#     # Устанавливаем библиотеку
#     install(TARGETS vk_storage
#         EXPORT vk_storageTargets
#         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     )
    
#     # Устанавливаем ТОЛЬКО публичные заголовки
#     install(DIRECTORY include/
#         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#         FILES_MATCHING PATTERN "*.h"
#     )
    
#     # НЕ устанавливаем приватные заголовки из src/
#     # Они остаются внутренними деталями реализации
    
#     # Экспортируем targets
#     install(EXPORT vk_storageTargets
#         FILE vk_storageTargets.cmake
#         NAMESPACE vk::
#         DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vk_storage
#     )
    
#     # Создаем config файл
#     include(CMakePackageConfigHelpers)
#     write_basic_package_version_file(
#         vk_storageConfigVersion.cmake
#         VERSION 1.0.0
#         COMPATIBILITY SameMajorVersion
#     )
    
#     configure_package_config_file(
#         ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vk_storageConfig.cmake.in
#         ${CMAKE_CURRENT_BINARY_DIR}/vk_storageConfig.cmake
#         INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vk_storage
#     )
    
#     install(FILES
#         ${CMAKE_CURRENT_BINARY_DIR}/vk_storageConfig.cmake
#         ${CMAKE_CURRENT_BINARY_DIR}/vk_storageConfigVersion.cmake
#         DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vk_storage
#     )
# endif()
