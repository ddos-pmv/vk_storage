add_executable(kv_client main.cpp client.cpp client.h
        client.h)

target_link_libraries(kv_client PRIVATE Boost::asio Boost::program_options)