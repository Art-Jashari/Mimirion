/**
 * @file test_commit.cpp
 * @brief Unit tests for the CommitManager class
 * @author Mimirion Team
 * @date June 4, 2025
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include "commit.hpp"

namespace fs = std::filesystem;

class CommitManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for each test
        testDir = fs::temp_directory_path() / "mimirion_test_commit";
        mimirionDir = testDir / ".mimirion";
        
        fs::create_directories(testDir);
        fs::create_directories(mimirionDir);
        fs::create_directories(mimirionDir / "objects");
        fs::create_directories(mimirionDir / "refs" / "heads");
        
        // Create HEAD file pointing to master branch
        std::ofstream headFile(mimirionDir / "HEAD");
        headFile << "ref: refs/heads/master" << std::endl;
        headFile.close();
        
        // Change to the test directory
        originalPath = fs::current_path();
        fs::current_path(testDir);
        
        // Initialize commit manager
        commitManager = std::make_unique<mimirion::CommitManager>(testDir, mimirionDir);
    }

    void TearDown() override {
        // Change back to the original directory
        fs::current_path(originalPath);
        
        // Clean up the temporary directory
        fs::remove_all(testDir);
        
        // Clean up the commit manager
        commitManager.reset();
    }
    
    // Create a sample file with content
    void createSampleFile(const std::string& name, const std::string& content) {
        std::ofstream file(testDir / name);
        file << content;
        file.close();
    }

    fs::path testDir;
    fs::path mimirionDir;
    fs::path originalPath;
    std::unique_ptr<mimirion::CommitManager> commitManager;
};

// Test creating a new commit
TEST_F(CommitManagerTest, CreateCommit) {
    // Create a sample file
    createSampleFile("test.txt", "Test file content");
    
    // Set up staged files
    std::vector<std::string> stagedFiles = {"test.txt"};
    
    // Create commit
    std::string commitHash = commitManager->createCommit("Test commit", stagedFiles);
    
    // Verify commit hash is not empty
    EXPECT_FALSE(commitHash.empty());
    
    // Verify commit exists
    mimirion::CommitInfo* commit = commitManager->getCommit(commitHash);
    EXPECT_NE(commit, nullptr);
    
    // Verify commit details
    if (commit != nullptr) {
        EXPECT_EQ(commit->message, "Test commit");
        EXPECT_TRUE(commit->fileHashes.find("test.txt") != commit->fileHashes.end());
    }
}

// Test commit history
TEST_F(CommitManagerTest, CommitHistory) {
    // Create a chain of commits
    std::vector<std::string> stagedFiles1 = {"file1.txt"};
    createSampleFile("file1.txt", "Content for file 1");
    std::string hash1 = commitManager->createCommit("First commit", stagedFiles1);
    
    std::vector<std::string> stagedFiles2 = {"file2.txt"};
    createSampleFile("file2.txt", "Content for file 2");
    std::string hash2 = commitManager->createCommit("Second commit", stagedFiles2);
    
    std::vector<std::string> stagedFiles3 = {"file3.txt"};
    createSampleFile("file3.txt", "Content for file 3");
    std::string hash3 = commitManager->createCommit("Third commit", stagedFiles3);
    
    // Get history
    std::vector<mimirion::CommitInfo> history = commitManager->getHistory();
    
    // Verify history length
    EXPECT_EQ(history.size(), 3);
    
    // Verify commit order (newest first)
    if (history.size() >= 3) {
        EXPECT_EQ(history[0].hash, hash3);
        EXPECT_EQ(history[1].hash, hash2);
        EXPECT_EQ(history[2].hash, hash1);
    }
}

// Test getting the HEAD commit
TEST_F(CommitManagerTest, GetHeadCommit) {
    // Create a commit
    std::vector<std::string> stagedFiles = {"head_test.txt"};
    createSampleFile("head_test.txt", "Test file for HEAD");
    std::string commitHash = commitManager->createCommit("HEAD commit test", stagedFiles);
    
    // Get HEAD commit
    mimirion::CommitInfo* headCommit = commitManager->getHeadCommit();
    
    // Verify HEAD commit
    EXPECT_NE(headCommit, nullptr);
    
    if (headCommit != nullptr) {
        EXPECT_EQ(headCommit->hash, commitHash);
        EXPECT_EQ(headCommit->message, "HEAD commit test");
    }
}

// Test commit parent-child relationships
TEST_F(CommitManagerTest, CommitParentRelationship) {
    // Create first commit
    std::vector<std::string> stagedFiles1 = {"parent.txt"};
    createSampleFile("parent.txt", "Parent file content");
    std::string parentHash = commitManager->createCommit("Parent commit", stagedFiles1);
    
    // Create child commit
    std::vector<std::string> stagedFiles2 = {"child.txt"};
    createSampleFile("child.txt", "Child file content");
    std::string childHash = commitManager->createCommit("Child commit", stagedFiles2);
    
    // Get child commit
    mimirion::CommitInfo* childCommit = commitManager->getCommit(childHash);
    
    // Verify parent-child relationship
    EXPECT_NE(childCommit, nullptr);
    
    if (childCommit != nullptr) {
        EXPECT_EQ(childCommit->parentHashes.size(), 1);
        
        if (childCommit->parentHashes.size() > 0) {
            EXPECT_EQ(childCommit->parentHashes[0], parentHash);
        }
    }
}
