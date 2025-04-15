set(current_target_name unrolled_list)

add_library(${current_target_name} INTERFACE)

target_include_directories(${current_target_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})