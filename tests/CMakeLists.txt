include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

add_executable(vk_storage_tests
    test_kv_storage.cpp
    test_thread_safety.cpp
    mock_clock.cpp
        test_kv_gossip.cpp
)

target_link_libraries(vk_storage_tests
    PRIVATE
    vk_storage
    gtest_main
    gtest
)

include(GoogleTest)
gtest_discover_tests(vk_storage_tests)