/**
 * @file test_main.cpp
 * @brief Main entry point for Mimirion tests
 * @author Mimirion Team
 * @date June 4, 2025
 */

#include <gtest/gtest.h>
#include <iostream>

/**
 * @brief Main function for running all tests
 * 
 * This is the entry point for the Mimirion test suite.
 * It initializes Google Test, runs all the tests, and
 * returns the result.
 */
int main(int argc, char **argv) {
    std::cout << "Running Mimirion test suite..." << std::endl;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
