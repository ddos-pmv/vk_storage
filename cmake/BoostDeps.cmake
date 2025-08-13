include(FetchContent)

option(USE_SYSTEM_BOOST "Use system-installed Boost" OFF)

function(setup_boost_dependencies)
    if(USE_SYSTEM_BOOST)
        find_package(Boost 1.70 QUIET)
        
        if(Boost_FOUND)
            message(STATUS "Using system Boost ${Boost_VERSION}")
            add_library(boost_intrusive_target ALIAS Boost::boost)
            return()
        else()
            message(STATUS "System Boost not found, falling back to FetchContent")
        endif()
    endif()

    message(STATUS "Using FetchContent for Boost.Intrusive")
    
    # Минимальный набор для Boost.Intrusive
    set(BOOST_VERSION "boost-1.83.0")
    
    FetchContent_Declare(
        boost_assert
        GIT_REPOSITORY https://github.com/boostorg/assert.git
        GIT_TAG ${BOOST_VERSION}
        GIT_SHALLOW TRUE
    )

    FetchContent_Declare(
        boost_config
        GIT_REPOSITORY https://github.com/boostorg/config.git
        GIT_TAG ${BOOST_VERSION}
        GIT_SHALLOW TRUE
    )

    FetchContent_Declare(
        boost_core
        GIT_REPOSITORY https://github.com/boostorg/core.git
        GIT_TAG ${BOOST_VERSION}
        GIT_SHALLOW TRUE
    )

    FetchContent_Declare(
        boost_move
        GIT_REPOSITORY https://github.com/boostorg/move.git
        GIT_TAG ${BOOST_VERSION}
        GIT_SHALLOW TRUE
    )

    FetchContent_Declare(
        boost_static_assert
        GIT_REPOSITORY https://github.com/boostorg/static_assert.git
        GIT_TAG ${BOOST_VERSION}
        GIT_SHALLOW TRUE
    )

    FetchContent_Declare(
        container_hash
        GIT_REPOSITORY https://github.com/boostorg/container_hash.git
        GIT_TAG ${BOOST_VERSION}
        GIT_SHALLOW TRUE
    )

    FetchContent_Declare(
        type_traits
        GIT_REPOSITORY https://github.com/boostorg/type_traits.git
        GIT_TAG ${BOOST_VERSION}
        GIT_SHALLOW TRUE
    )

    FetchContent_Declare(
        boost_intrusive
        GIT_REPOSITORY https://github.com/boostorg/intrusive.git
        GIT_TAG ${BOOST_VERSION}
        GIT_SHALLOW TRUE
    )

    # Загружаем все компоненты
    FetchContent_MakeAvailable(
        boost_assert
        boost_config 
        boost_core
        boost_move
        boost_static_assert
        container_hash
        type_traits
        boost_intrusive
    )

    # Создаем unified target
    add_library(boost_intrusive_target INTERFACE)
    target_include_directories(boost_intrusive_target INTERFACE
        ${boost_assert_SOURCE_DIR}/include
        ${boost_config_SOURCE_DIR}/include
        ${boost_core_SOURCE_DIR}/include
        ${boost_move_SOURCE_DIR}/include
        ${boost_static_assert_SOURCE_DIR}/include
        ${container_hash_SOURCE_DIR}/include
        ${type_traits_SOURCE_DIR}/include
        ${boost_intrusive_SOURCE_DIR}/include
    )
    
    message(STATUS "Boost.Intrusive FetchContent setup complete")
endfunction()