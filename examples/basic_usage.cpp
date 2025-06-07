#include "../include/repository.hpp"
#include "../include/github_api.hpp"
#include <iostream>
#include <string>

/**
 * @file basic_usage.cpp
 * @brief Example demonstrating basic usage of Mimirion VCS
 * 
 * This example shows how to initialize a repository, add files,
 * create commits, branches, and interact with GitHub.
 */

int main(int argc, char* argv[]) {
    // Create repository instance
    mimirion::Repository repo;
    
    // Initialize new repository in the current directory
    if (!repo.init(".")) {
        std::cerr << "Failed to initialize repository" << std::endl;
        return 1;
    }
    
    std::cout << "Repository initialized successfully" << std::endl;
    
    // Create a sample README file
    std::ofstream readme("README.md");
    if (readme) {
        readme << "# Sample Project\n\n";
        readme << "This is a sample project managed with Mimirion VCS.\n";
        readme.close();
        
        // Add the file to staging
        if (repo.add("README.md")) {
            std::cout << "Added README.md to staging area" << std::endl;
            
            // Create initial commit
            std::string commitHash = repo.commit("Initial commit");
            if (!commitHash.empty()) {
                std::cout << "Created initial commit: " << commitHash << std::endl;
            } else {
                std::cerr << "Failed to create commit" << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Failed to add file" << std::endl;
            return 1;
        }
    }
    
    // Create a branch
    if (repo.createBranch("feature")) {
        std::cout << "Created 'feature' branch" << std::endl;
        
        // Switch to the branch
        if (repo.checkout("feature")) {
            std::cout << "Switched to 'feature' branch" << std::endl;
            
            // Make changes on the feature branch
            std::ofstream featureFile("feature.txt");
            if (featureFile) {
                featureFile << "This file was added in the feature branch.\n";
                featureFile.close();
                
                // Add and commit the new file
                if (repo.add("feature.txt")) {
                    std::string featureCommitHash = repo.commit("Add feature file");
                    if (!featureCommitHash.empty()) {
                        std::cout << "Created feature commit: " << featureCommitHash << std::endl;
                    }
                }
            }
            
            // Switch back to master
            repo.checkout("master");
            std::cout << "Switched back to 'master' branch" << std::endl;
        }
    }
    
    // Optional: Setup GitHub integration
    if (argc > 2) {
        std::string username = argv[1];
        std::string token = argv[2];
        
        mimirion::GitHubProvider github;
        github.setCredentials(username, token);
        
        std::cout << "Creating GitHub repository..." << std::endl;
        if (github.createRepository("sample-mimirion-repo", "Sample repo created with Mimirion", false)) {
            std::cout << "GitHub repository created" << std::endl;
            
            // Add remote
            repo.addRemote("origin", "https://github.com/" + username + "/sample-mimirion-repo.git");
            
            // Push to GitHub
            if (repo.push("origin", "master")) {
                std::cout << "Pushed to GitHub successfully" << std::endl;
            } else {
                std::cerr << "Failed to push to GitHub" << std::endl;
            }
        } else {
            std::cerr << "Failed to create GitHub repository" << std::endl;
        }
    }
    
    std::cout << "Example completed successfully" << std::endl;
    return 0;
}
