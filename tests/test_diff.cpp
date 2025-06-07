/**
 * @file test_diff.cpp
 * @brief Unit tests for the DiffEngine class
 * @author Mimirion Team
 * @date June 4, 2025
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include "diff.hpp"

namespace fs = std::filesystem;

class DiffEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for each test
        testDir = fs::temp_directory_path() / "mimirion_test_diff";
        fs::create_directories(testDir);
        
        // Change to the test directory
        originalPath = fs::current_path();
        fs::current_path(testDir);
        
        // Initialize diff engine
        diffEngine = std::make_unique<mimirion::DiffEngine>();
    }

    void TearDown() override {
        // Change back to the original directory
        fs::current_path(originalPath);
        
        // Clean up the temporary directory
        fs::remove_all(testDir);
        
        // Clean up the diff engine
        diffEngine.reset();
    }
    
    // Create a sample file with content
    std::string createSampleFile(const std::string& name, const std::string& content) {
        fs::path filePath = testDir / name;
        std::ofstream file(filePath);
        file << content;
        file.close();
        return filePath.string();
    }

    fs::path testDir;
    fs::path originalPath;
    std::unique_ptr<mimirion::DiffEngine> diffEngine;
};

// Test generating diff between identical files
TEST_F(DiffEngineTest, IdenticalFiles) {
    std::string content = "Line 1\nLine 2\nLine 3\n";
    std::string file1 = createSampleFile("file1.txt", content);
    std::string file2 = createSampleFile("file2.txt", content);
    
    mimirion::FileDiff diff = diffEngine->generateDiff(file1, file2);
    
    // No hunks should be generated for identical files
    EXPECT_EQ(diff.hunks.size(), 0);
}

// Test generating diff with added lines
TEST_F(DiffEngineTest, AddedLines) {
    std::string content1 = "Line 1\nLine 2\nLine 3\n";
    std::string content2 = "Line 1\nLine 2\nNew Line\nLine 3\n";
    
    std::string file1 = createSampleFile("original.txt", content1);
    std::string file2 = createSampleFile("modified.txt", content2);
    
    mimirion::FileDiff diff = diffEngine->generateDiff(file1, file2);
    
    // Should have one hunk
    EXPECT_EQ(diff.hunks.size(), 1);
    
    // Verify the hunk details
    if (diff.hunks.size() > 0) {
        EXPECT_EQ(diff.hunks[0].oldCount, 2); // Lines 2-3
        EXPECT_EQ(diff.hunks[0].newCount, 3); // Lines 2-4 (added new line)
    }
}

// Test generating diff with removed lines
TEST_F(DiffEngineTest, RemovedLines) {
    std::string content1 = "Line 1\nLine 2\nLine 3\nLine 4\n";
    std::string content2 = "Line 1\nLine 4\n";
    
    std::string file1 = createSampleFile("original.txt", content1);
    std::string file2 = createSampleFile("modified.txt", content2);
    
    mimirion::FileDiff diff = diffEngine->generateDiff(file1, file2);
    
    // Should have one hunk
    EXPECT_EQ(diff.hunks.size(), 1);
    
    // Verify the hunk details
    if (diff.hunks.size() > 0) {
        EXPECT_EQ(diff.hunks[0].oldCount, 4); // Lines 1-4
        EXPECT_EQ(diff.hunks[0].newCount, 2); // Lines 1,4 (removed lines 2-3)
    }
}

// Test generating diff with modified lines
TEST_F(DiffEngineTest, ModifiedLines) {
    std::string content1 = "Line 1\nLine 2\nLine 3\n";
    std::string content2 = "Line 1\nModified Line 2\nLine 3\n";
    
    std::string file1 = createSampleFile("original.txt", content1);
    std::string file2 = createSampleFile("modified.txt", content2);
    
    mimirion::FileDiff diff = diffEngine->generateDiff(file1, file2);
    
    // Should have one hunk
    EXPECT_EQ(diff.hunks.size(), 1);
    
    // Verify the hunk details
    if (diff.hunks.size() > 0) {
        EXPECT_EQ(diff.hunks[0].oldCount, 3); // Lines 1-3 (Line 2 modified)
        EXPECT_EQ(diff.hunks[0].newCount, 3); // Lines 1-3
    }
}

// Test generating diff from strings
TEST_F(DiffEngineTest, DiffFromStrings) {
    std::string content1 = "Line 1\nLine 2\nLine 3\n";
    std::string content2 = "Line 1\nLine 2\nLine 3\nLine 4\n";
    
    mimirion::FileDiff diff = diffEngine->generateDiffFromStrings(content1, content2);
    
    // Should have one hunk
    EXPECT_EQ(diff.hunks.size(), 1);
    
    // Verify the hunk contains added line
    if (diff.hunks.size() > 0) {
        bool foundAddedLine = false;
        for (const auto& line : diff.hunks[0].lines) {
            if (line == "+Line 4") {
                foundAddedLine = true;
                break;
            }
        }
        EXPECT_TRUE(foundAddedLine);
    }
}

// Test applying a diff patch
TEST_F(DiffEngineTest, ApplyDiff) {
    std::string content1 = "Line 1\nLine 2\nLine 3\n";
    std::string content2 = "Line 1\nModified Line 2\nLine 3\nNew Line 4\n";
    
    std::string file1 = createSampleFile("original.txt", content1);
    
    // Generate diff
    mimirion::FileDiff diff = diffEngine->generateDiffFromStrings(content1, content2);
    
    // Apply diff to the original file
    EXPECT_TRUE(diffEngine->applyDiff(diff, file1));
    
    // Read the patched file
    std::ifstream patchedFile(file1);
    std::stringstream buffer;
    buffer << patchedFile.rdbuf();
    std::string patchedContent = buffer.str();
    
    // Verify the content matches
    EXPECT_EQ(patchedContent, content2);
}
