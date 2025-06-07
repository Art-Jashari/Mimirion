cmake_minimum_required(VERSION 3.13)
project(Mimirion VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to build tests
option(MIMIRION_BUILD_TESTS "Build the Mimirion test suite" ON)

# Enable warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
elseif(MSVC)
  add_compile_options(/W4)
endif()

# Add include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add source files
set(SOURCES
    src/main.cpp
    src/repository.cpp
    src/file_tracker.cpp
    src/commit.cpp
    src/diff.cpp
    src/remote.cpp
    src/github_api.cpp
    src/utils.cpp
)

# Create executable
add_executable(mimirion ${SOURCES})

# Find and link required libraries
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
target_link_libraries(mimirion PRIVATE CURL::libcurl OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB)

# Install target
install(TARGETS mimirion DESTINATION bin)

# Build examples
set(LIB_SOURCES
    src/repository.cpp
    src/file_tracker.cpp
    src/commit.cpp
    src/diff.cpp
    src/remote.cpp
    src/github_api.cpp
    src/utils.cpp
)
add_executable(github_example examples/github_example.cpp ${LIB_SOURCES})
target_link_libraries(github_example PRIVATE CURL::libcurl OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB)
install(TARGETS github_example DESTINATION bin)

# Google Test setup
if(MIMIRION_BUILD_TESTS)
    # Fetch and build Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    # Enable testing
    enable_testing()
    
    # Include tests directory
    add_subdirectory(tests)
endif()
