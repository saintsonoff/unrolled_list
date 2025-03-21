cmake_minimum_required(VERSION 3.30)

project(
    labwork7
    LANGUAGES CXX
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(lib)

add_subdirectory(bin)

enable_testing()
add_subdirectory(tests)
