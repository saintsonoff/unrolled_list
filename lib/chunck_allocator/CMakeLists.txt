set(current_target_name chunck_allocator)

add_library(${current_target_name} INTERFACE)

target_include_directories(${current_target_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})