cmake_minimum_required(VERSION 3.10)

project(LinkMsg
    VERSION 0.1.0
    DESCRIPTION "A lightweight C++ publish-subscribe message bus"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


option(LINKMSG_BUILD_EXAMPLES "Build examples" ON)


add_library(linkmsg
    src/bus/MessageBus.cpp
)


target_include_directories(linkmsg
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)


target_compile_features(linkmsg PUBLIC cxx_std_11)


if (LINKMSG_BUILD_EXAMPLES)
    add_executable(example_basic
        examples/basic_test.cpp
    )
    target_link_libraries(example_basic
        PRIVATE linkmsg
    )
endif()
