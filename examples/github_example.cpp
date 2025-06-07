#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <ctime>
#include "../include/repository.hpp"

/**
 * @file github_example.cpp
 * @brief Example showing how to push to GitHub with Mimirion
 * @author Mimirion Team
 * @date June 7, 2025
 */

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <github_username> <github_token> <repository_path>" << std::endl;
        return 1;
    }
    
    std::string username = argv[1];
    std::string token = argv[2];
    std::filesystem::path repoPath = argv[3];
    
    // Initialize Mimirion repository
    mimirion::Repository repo;
    
    // Load or initialize the repository
    if (!std::filesystem::exists(repoPath / ".mimirion")) {
        std::cout << "Initializing new repository..." << std::endl;
        if (!repo.init(repoPath)) {
            std::cerr << "Failed to initialize repository" << std::endl;
            return 1;
        }
    } else {
        std::cout << "Loading existing repository..." << std::endl;
        if (!repo.load(repoPath)) {
            std::cerr << "Failed to load repository" << std::endl;
            return 1;
        }
    }
    
    // Set GitHub credentials
    if (!repo.setGitHubCredentials(username, token)) {
        std::cerr << "Failed to set GitHub credentials" << std::endl;
        return 1;
    }
    
    // Add a remote repository
    std::string remoteUrl = "https://github.com/" + username + "/mimirion-test.git";
    if (!repo.addRemote("origin", remoteUrl)) {
        std::cerr << "Failed to add remote" << std::endl;
        return 1;
    }
    
    // Create a test file
    std::string testFilePath = (repoPath / "README.md").string();
    std::ofstream testFile(testFilePath);
    testFile << "# Mimirion Test Repository\n\n"
             << "This repository is used to test the Mimirion VCS GitHub integration.\n"
             << "Current date: " << std::time(nullptr) << "\n";
    testFile.close();
    
    // Stage the file
    if (!repo.add("README.md")) {
        std::cerr << "Failed to add file" << std::endl;
        return 1;
    }
    
    // Create a commit
    std::string commitHash = repo.commit("Initial commit");
    if (commitHash.empty()) {
        std::cerr << "Failed to create commit" << std::endl;
        return 1;
    }
    
    // Push to GitHub
    if (!repo.push("origin", "master")) {
        std::cerr << "Failed to push to GitHub" << std::endl;
        return 1;
    }
    
    std::cout << "Successfully pushed to GitHub!" << std::endl;
    
    return 0;
}
