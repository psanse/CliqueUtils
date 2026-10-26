#######################
# CliqueUtils unit tests
#######################

add_executable(tests_clq_func
    test_clique_utils.cpp
)

# Link dependencies
target_link_libraries(tests_clq_func
    PRIVATE
        graph
        bitscan
        utils
        GTest::gtest_main
)

# Include directories
target_include_directories(tests_clq_func
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${bitgraph_SOURCE_DIR}/src
        ${bitgraph_SOURCE_DIR}/src/graph/algorithms
)

# Require C++14
target_compile_features(tests_clq_func
    PRIVATE cxx_std_14
)

# Output directory
set_target_properties(tests_clq_func
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests"
)

# Register GoogleTest tests with CTest
include(GoogleTest)
gtest_discover_tests(tests_clq_func)
