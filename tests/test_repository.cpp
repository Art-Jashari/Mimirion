/**
 * @file test_repository.cpp
 * @brief Unit tests for the Repository class
 * @author Mimirion Team
 * @date June 4, 2025
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include "repository.hpp"

namespace fs = std::filesystem;

class RepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for each test
        testDir = fs::temp_directory_path() / "mimirion_test_repo";
        fs::create_directories(testDir);
        
        // Change to the test directory
        originalPath = fs::current_path();
        fs::current_path(testDir);
    }

    void TearDown() override {
        // Change back to the original directory
        fs::current_path(originalPath);
        
        // Clean up the temporary directory
        fs::remove_all(testDir);
    }
    
    // Create a sample file with contentfile:///home/shllaki/Repositories/Mimirion/docs/html/index.html
    void createSampleFile(const std::string& name, const std::string& content) {
        std::ofstream file(testDir / name);
        file << content;
        file.close();
    }

    fs::path testDir;
    fs::path originalPath;
};

// Test repository initialization
TEST_F(RepositoryTest, InitializeRepository) {
    mimirion::Repository repo;
    
    // Test initialization
    EXPECT_TRUE(repo.init(testDir.string()));
    
    // Check if .mimirion directory was created
    EXPECT_TRUE(fs::exists(testDir / ".mimirion"));
    EXPECT_TRUE(fs::exists(testDir / ".mimirion" / "HEAD"));
    EXPECT_TRUE(fs::exists(testDir / ".mimirion" / "objects"));
    EXPECT_TRUE(fs::exists(testDir / ".mimirion" / "refs" / "heads"));
}

// Test adding files to the repository
TEST_F(RepositoryTest, AddFiles) {
    mimirion::Repository repo;
    repo.init(testDir.string());
    
    // Create a sample file
    createSampleFile("test.txt", "This is a test file content");
    
    // Add the file to the repository
    EXPECT_TRUE(repo.add("test.txt"));
    
    // Check the status to ensure the file is staged
    std::string status = repo.status();
    EXPECT_TRUE(status.find("test.txt") != std::string::npos);
}

// Test creating a commit
TEST_F(RepositoryTest, CreateCommit) {
    mimirion::Repository repo;
    repo.init(testDir.string());
    
    // Create and add a sample file
    createSampleFile("commit_test.txt", "This file will be committed");
    repo.add("commit_test.txt");
    
    // Create a commit
    std::string commitHash = repo.commit("Initial commit for testing");
    
    // Verify that a commit hash was returned
    EXPECT_FALSE(commitHash.empty());
}

// Test creating and switching branches
TEST_F(RepositoryTest, BranchAndCheckout) {
    mimirion::Repository repo;
    repo.init(testDir.string());
    
    // Create an initial commit
    createSampleFile("main_file.txt", "This is on main branch");
    repo.add("main_file.txt");
    repo.commit("Initial commit on master");
    
    // Create a new branch
    EXPECT_TRUE(repo.createBranch("test-branch"));
    
    // Switch to the new branch
    EXPECT_TRUE(repo.checkout("test-branch"));
    
    // Create a file in the new branch
    createSampleFile("branch_file.txt", "This is on test-branch");
    repo.add("branch_file.txt");
    repo.commit("Commit on test branch");
    
    // Switch back to master
    EXPECT_TRUE(repo.checkout("master"));
    
    // Verify that status shows we're on master
    std::string status = repo.status();
    EXPECT_TRUE(status.find("On branch master") != std::string::npos);
}
