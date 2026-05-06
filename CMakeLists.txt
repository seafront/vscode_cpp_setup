cmake_minimum_required(VERSION 3.20)
project(MyProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Pre-built googletest v1.15.2 (ClangCL, Debug)
foreach(_lib gtest gtest_main gmock gmock_main)
    add_library(${_lib} STATIC IMPORTED)
    set_target_properties(${_lib} PROPERTIES
        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/gtest/lib/${_lib}.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lib/gtest/include"
    )
endforeach()

enable_testing()

add_executable(tests
    tests/test_main.cpp
    src/nvme_device.cpp
)
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(tests gtest_main gmock)

include(GoogleTest)
gtest_discover_tests(tests)
