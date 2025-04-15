add_executable(
    chunck-allocator-lib-tests
    allocator_ut.cpp
    exception_safety_ut.cpp
    named_requirements_ut.cpp
    no_default_constructible_ut.cpp
    simple_ut.cpp
)

target_link_libraries(
    chunck-allocator-lib-tests
    GTest::gtest_main
    GTest::gmock_main

    chunck_allocator
)

target_include_directories(chunck-allocator-lib-tests PUBLIC ${PROJECT_SOURCE_DIR})

include(GoogleTest)

gtest_discover_tests(chunck-allocator-lib-tests)
