set(current_target_name ${PROJECT_NAME})

add_executable(${current_target_name} main.cpp)

target_link_libraries(${current_target_name}
  PUBLIC
    unrolled_list
)
