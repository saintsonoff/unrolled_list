enable_testing()

add_executable(
    unrolled-list-lib-tests
    allocator_ut.cpp
    exception_safety_ut.cpp
    named_requirements_ut.cpp
    no_default_constructible_ut.cpp
    simple_ut.cpp
)

target_link_libraries(
    unrolled-list-lib-tests
    GTest::gtest_main
    GTest::gmock_main

    unrolled_list
)

target_include_directories(unrolled-list-lib-tests PUBLIC ${PROJECT_SOURCE_DIR})

include(GoogleTest)

gtest_discover_tests(unrolled-list-lib-tests)
