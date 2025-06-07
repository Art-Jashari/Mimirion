/**
 * @file test_remote.cpp
 * @brief Unit tests for the RemoteManager class
 * @author Mimirion Team
 * @date June 4, 2025
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include "remote.hpp"

namespace fs = std::filesystem;

class RemoteTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for each test
        testDir = fs::temp_directory_path() / "mimirion_test_remote";
        mimirionDir = testDir / ".mimirion";
        
        fs::create_directories(testDir);
        fs::create_directories(mimirionDir);
        fs::create_directories(mimirionDir / "refs" / "remotes");
        
        // Change to the test directory
        originalPath = fs::current_path();
        fs::current_path(testDir);
        
        // Initialize remote manager
        remoteManager = std::make_unique<mimirion::RemoteManager>(testDir, mimirionDir);
    }

    void TearDown() override {
        // Change back to the original directory
        fs::current_path(originalPath);
        
        // Clean up the temporary directory
        fs::remove_all(testDir);
        
        // Clean up the remote manager
        remoteManager.reset();
    }

    fs::path testDir;
    fs::path mimirionDir;
    fs::path originalPath;
    std::unique_ptr<mimirion::RemoteManager> remoteManager;
};

// Test adding a remote repository
TEST_F(RemoteTest, AddRemote) {
    // Add a remote repository
    std::string remoteName = "origin";
    std::string remoteUrl = "https://github.com/user/repo.git";
    
    EXPECT_TRUE(remoteManager->addRemote(remoteName, remoteUrl));
    
    // Verify remote was added
    auto remotes = remoteManager->getRemotes();
    EXPECT_EQ(remotes.size(), 1);
    
    // Check remote details
    if (remotes.size() > 0) {
        EXPECT_TRUE(remotes.find(remoteName) != remotes.end());
        EXPECT_EQ(remotes[remoteName], remoteUrl);
    }
}

// Test adding multiple remotes
TEST_F(RemoteTest, MultipleRemotes) {
    // Add multiple remotes
    EXPECT_TRUE(remoteManager->addRemote("origin", "https://github.com/user/repo.git"));
    EXPECT_TRUE(remoteManager->addRemote("upstream", "https://github.com/upstream/repo.git"));
    EXPECT_TRUE(remoteManager->addRemote("custom", "https://gitlab.com/user/repo.git"));
    
    // Verify remotes were added
    auto remotes = remoteManager->getRemotes();
    EXPECT_EQ(remotes.size(), 3);
    
    // Check remote details
    EXPECT_EQ(remotes["origin"], "https://github.com/user/repo.git");
    EXPECT_EQ(remotes["upstream"], "https://github.com/upstream/repo.git");
    EXPECT_EQ(remotes["custom"], "https://gitlab.com/user/repo.git");
}

// Test removing a remote
TEST_F(RemoteTest, RemoveRemote) {
    // Add remotes
    EXPECT_TRUE(remoteManager->addRemote("origin", "https://github.com/user/repo.git"));
    EXPECT_TRUE(remoteManager->addRemote("upstream", "https://github.com/upstream/repo.git"));
    
    // Verify remotes were added
    auto initialRemotes = remoteManager->getRemotes();
    EXPECT_EQ(initialRemotes.size(), 2);
    
    // Remove a remote
    EXPECT_TRUE(remoteManager->removeRemote("origin"));
    
    // Verify remote was removed
    auto updatedRemotes = remoteManager->getRemotes();
    EXPECT_EQ(updatedRemotes.size(), 1);
    EXPECT_TRUE(updatedRemotes.find("origin") == updatedRemotes.end());
    EXPECT_TRUE(updatedRemotes.find("upstream") != updatedRemotes.end());
}

// Test saving and loading remote state
TEST_F(RemoteTest, SaveAndLoadState) {
    // Add remotes
    EXPECT_TRUE(remoteManager->addRemote("origin", "https://github.com/user/repo.git"));
    EXPECT_TRUE(remoteManager->addRemote("upstream", "https://github.com/upstream/repo.git"));
    
    // Save state
    EXPECT_TRUE(remoteManager->saveState());
    
    // Create a new remote manager (simulates program restart)
    remoteManager.reset();
    remoteManager = std::make_unique<mimirion::RemoteManager>(testDir, mimirionDir);
    
    // Load state
    EXPECT_TRUE(remoteManager->loadState());
    
    // Verify remotes are still there
    auto remotes = remoteManager->getRemotes();
    EXPECT_EQ(remotes.size(), 2);
    EXPECT_EQ(remotes["origin"], "https://github.com/user/repo.git");
    EXPECT_EQ(remotes["upstream"], "https://github.com/upstream/repo.git");
}

// Test getting URL for a specific remote
TEST_F(RemoteTest, GetRemoteUrl) {
    // Add a remote
    std::string remoteName = "origin";
    std::string remoteUrl = "https://github.com/user/repo.git";
    EXPECT_TRUE(remoteManager->addRemote(remoteName, remoteUrl));
    
    // Get all remotes and check if our remote exists with the right URL
    auto remotes = remoteManager->getRemotes();
    EXPECT_TRUE(remotes.find(remoteName) != remotes.end());
    EXPECT_EQ(remotes[remoteName], remoteUrl);
    
    // Test for non-existent remote
    EXPECT_TRUE(remotes.find("non-existent") == remotes.end());
}
