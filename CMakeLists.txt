cmake_minimum_required(VERSION 3.16)

project(meow VERSION 0.1.0 LANGUAGES CXX)

# CMake variables
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# 3rd party libraries
find_package(asio CONFIG REQUIRED)

add_library(meow STATIC)
target_include_directories(meow PUBLIC src)
add_subdirectory(src)
target_link_libraries(meow PUBLIC asio::asio)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    add_subdirectory(tests)
endif()