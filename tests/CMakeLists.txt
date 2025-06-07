# Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.13)

# Create library for everything except main.cpp
set(MIMIRION_LIB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/repository.cpp
    ${CMAKE_SOURCE_DIR}/src/file_tracker.cpp
    ${CMAKE_SOURCE_DIR}/src/commit.cpp
    ${CMAKE_SOURCE_DIR}/src/diff.cpp
    ${CMAKE_SOURCE_DIR}/src/remote.cpp
    ${CMAKE_SOURCE_DIR}/src/github_api.cpp
    ${CMAKE_SOURCE_DIR}/src/utils.cpp
)

# Create the library that will be used by tests
add_library(mimirion_lib STATIC ${MIMIRION_LIB_SOURCES})
target_link_libraries(mimirion_lib PRIVATE CURL::libcurl OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB)

# Unit test sources
set(TEST_SOURCES
    test_repository.cpp
    test_file_tracker.cpp
    test_commit.cpp
    test_diff.cpp
    test_remote.cpp
    test_utils.cpp
    test_main.cpp
)

# Create test executable
add_executable(mimirion_tests ${TEST_SOURCES})
target_link_libraries(mimirion_tests 
    PRIVATE
    mimirion_lib
    gtest
    gtest_main
    CURL::libcurl
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
)

# Add tests to CTest
add_test(NAME mimirion_unit_tests COMMAND mimirion_tests)

# Integration test sources
set(INTEGRATION_TEST_SOURCES
    integration_tests.cpp
    test_main.cpp
)

# Create integration test executable
add_executable(mimirion_integration_tests ${INTEGRATION_TEST_SOURCES})
target_link_libraries(mimirion_integration_tests 
    PRIVATE
    mimirion_lib
    gtest
    gtest_main
    CURL::libcurl
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
)

# Add integration tests to CTest
add_test(NAME mimirion_integration_tests COMMAND mimirion_integration_tests)
