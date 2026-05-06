file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/integration/*.cpp"
)

add_executable(CFE_tests ${TEST_SOURCES})
cfe_target_options(CFE_tests)

target_link_libraries(CFE_tests PRIVATE
    CFE_lib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(CFE_tests)