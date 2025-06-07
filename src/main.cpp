#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <functional>
#include "../include/repository.hpp"
#include "../include/github_api.hpp"

// Main program for Mimirion VCS
// A custom version control system with GitHub integration

namespace fs = std::filesystem;

void printUsage() {
    std::cout << "Mimirion - Custom Version Control System\n"
              << "Usage: mimirion <command> [<args>]\n\n"
              << "Commands:\n"
              << "  init                Initialize a new repository\n"
              << "  status              Show repository status\n"
              << "  add <path>          Add file(s) to staging area\n"
              << "  commit <message>    Commit staged changes\n"
              << "  log                 Show commit history\n"
              << "  branch <name>       Create a new branch\n"
              << "  checkout <name>     Switch to a branch\n"
              << "  remote add <name> <url>  Add a remote repository\n"
              << "  remote list         List remote repositories\n"
              << "  push [<remote>] [<branch>]  Push to a remote repository\n"
              << "  pull [<remote>] [<branch>]  Pull from a remote repository\n"
              << "  github login        Set GitHub credentials\n"
              << "  github create <name> Create a new GitHub repository\n"
              << "  help                Show this help message\n"
              << std::endl;
}

int main(int argc, char** argv) {
    // Check if any command was provided
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];
    
    // Create repository instance
    mimirion::Repository repo;
    
    // Create GitHub API instance
    mimirion::GitHubProvider github;
    
    // Command handlers
    if (command == "init") {
        // Initialize a new repository
        std::string path = ".";
        if (argc > 2) {
            path = argv[2];
        }
        
        if (repo.init(path)) {
            std::cout << "Initialized empty Mimirion repository in " << fs::absolute(path) << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to initialize repository" << std::endl;
            return 1;
        }
    } 
    else if (command == "status") {
        // Load repository
        if (!repo.load(".")) {
            std::cerr << "Not a Mimirion repository" << std::endl;
            return 1;
        }
        
        std::cout << repo.status() << std::endl;
        return 0;
    }
    else if (command == "add") {
        // Check if path argument is provided
        if (argc < 3) {
            std::cerr << "Missing path argument" << std::endl;
            return 1;
        }
        
        // Load repository
        if (!repo.load(".")) {
            std::cerr << "Not a Mimirion repository" << std::endl;
            return 1;
        }
        
        // Add file or directory
        std::string path = argv[2];
        if (repo.add(path)) {
            std::cout << "Added " << path << " to stage" << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to add " << path << std::endl;
            return 1;
        }
    }
    else if (command == "commit") {
        // Check if message argument is provided
        if (argc < 3) {
            std::cerr << "Missing commit message" << std::endl;
            return 1;
        }
        
        // Load repository
        if (!repo.load(".")) {
            std::cerr << "Not a Mimirion repository" << std::endl;
            return 1;
        }
        
        // Commit changes
        std::string message = argv[2];
        std::string commitHash = repo.commit(message);
        if (!commitHash.empty()) {
            std::cout << "Committed changes [" << commitHash.substr(0, 8) << "]: " << message << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to commit changes" << std::endl;
            return 1;
        }
    }
    else if (command == "branch") {
        // Load repository
        if (!repo.load(".")) {
            std::cerr << "Not a Mimirion repository" << std::endl;
            return 1;
        }
        
        if (argc < 3) {
            // TODO: List branches
            std::cout << "TODO: List branches" << std::endl;
            return 0;
        } else {
            // Create branch
            std::string name = argv[2];
            if (repo.createBranch(name)) {
                std::cout << "Created branch " << name << std::endl;
                return 0;
            } else {
                std::cerr << "Failed to create branch " << name << std::endl;
                return 1;
            }
        }
    }
    else if (command == "checkout") {
        // Check if branch name is provided
        if (argc < 3) {
            std::cerr << "Missing branch name" << std::endl;
            return 1;
        }
        
        // Load repository
        if (!repo.load(".")) {
            std::cerr << "Not a Mimirion repository" << std::endl;
            return 1;
        }
        
        // Switch to branch
        std::string name = argv[2];
        if (repo.checkout(name)) {
            std::cout << "Switched to branch " << name << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to switch to branch " << name << std::endl;
            return 1;
        }
    }
    else if (command == "remote") {
        // Check if subcommand is provided
        if (argc < 3) {
            std::cerr << "Missing remote subcommand" << std::endl;
            return 1;
        }
        
        // Load repository
        if (!repo.load(".")) {
            std::cerr << "Not a Mimirion repository" << std::endl;
            return 1;
        }
        
        std::string subcommand = argv[2];
        if (subcommand == "add") {
            // Check if name and URL arguments are provided
            if (argc < 5) {
                std::cerr << "Missing remote name or URL" << std::endl;
                return 1;
            }
            
            // Add remote
            std::string name = argv[3];
            std::string url = argv[4];
            if (repo.addRemote(name, url)) {
                std::cout << "Added remote " << name << " at " << url << std::endl;
                return 0;
            } else {
                std::cerr << "Failed to add remote " << name << std::endl;
                return 1;
            }
        }
        else if (subcommand == "list") {
            // TODO: List remotes
            std::cout << "TODO: List remotes" << std::endl;
            return 0;
        }
        else {
            std::cerr << "Unknown remote subcommand: " << subcommand << std::endl;
            return 1;
        }
    }
    else if (command == "push") {
        // Load repository
        if (!repo.load(".")) {
            std::cerr << "Not a Mimirion repository" << std::endl;
            return 1;
        }
        
        // Get remote and branch arguments
        std::string remote = "origin";
        std::string branch = "";
        if (argc > 2) {
            remote = argv[2];
        }
        if (argc > 3) {
            branch = argv[3];
        }
        
        // Push changes
        if (repo.push(remote, branch)) {
            std::cout << "Pushed changes to " << remote;
            if (!branch.empty()) {
                std::cout << "/" << branch;
            }
            std::cout << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to push changes" << std::endl;
            return 1;
        }
    }
    else if (command == "pull") {
        // Load repository
        if (!repo.load(".")) {
            std::cerr << "Not a Mimirion repository" << std::endl;
            return 1;
        }
        
        // Get remote and branch arguments
        std::string remote = "origin";
        std::string branch = "";
        if (argc > 2) {
            remote = argv[2];
        }
        if (argc > 3) {
            branch = argv[3];
        }
        
        // Pull changes
        if (repo.pull(remote, branch)) {
            std::cout << "Pulled changes from " << remote;
            if (!branch.empty()) {
                std::cout << "/" << branch;
            }
            std::cout << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to pull changes" << std::endl;
            return 1;
        }
    }
    else if (command == "github") {
        // Check if subcommand is provided
        if (argc < 3) {
            std::cerr << "Missing GitHub subcommand" << std::endl;
            return 1;
        }
        
        std::string subcommand = argv[2];
        if (subcommand == "login") {
            // Get GitHub credentials
            std::string username, token;
            std::cout << "GitHub Username: ";
            std::getline(std::cin, username);
            std::cout << "GitHub Personal Access Token: ";
            std::getline(std::cin, token);
            
            // Set credentials
            github.setCredentials(username, token);
            
            // Save credentials
            fs::path home = getenv("HOME") ? getenv("HOME") : ".";
            fs::path credFile = home / ".mimirion" / "github_credentials";
            if (github.saveCredentialsToFile(credFile)) {
                std::cout << "GitHub credentials saved" << std::endl;
                return 0;
            } else {
                std::cerr << "Failed to save GitHub credentials" << std::endl;
                return 1;
            }
        }
        else if (subcommand == "create") {
            // Check if repository name is provided
            if (argc < 4) {
                std::cerr << "Missing repository name" << std::endl;
                return 1;
            }
            
            // Load GitHub credentials
            fs::path home = getenv("HOME") ? getenv("HOME") : ".";
            fs::path credFile = home / ".mimirion" / "github_credentials";
            if (!github.setCredentialsFromFile(credFile)) {
                std::cerr << "Failed to load GitHub credentials. Please run 'mimirion github login' first." << std::endl;
                return 1;
            }
            
            // Create repository
            std::string name = argv[3];
            std::string description = argc > 4 ? argv[4] : "";
            bool isPrivate = argc > 5 && std::string(argv[5]) == "private";
            
            std::string repoUrl = github.createRepository(name, description, isPrivate);
            if (!repoUrl.empty()) {
                std::cout << "Created GitHub repository: " << repoUrl << std::endl;
                
                // Load repository if exists
                if (repo.load(".")) {
                    // Add remote
                    if (repo.addRemote("origin", repoUrl)) {
                        std::cout << "Added remote 'origin' pointing to the new repository" << std::endl;
                    }
                }
                
                return 0;
            } else {
                std::cerr << "Failed to create GitHub repository" << std::endl;
                return 1;
            }
        }
        else {
            std::cerr << "Unknown GitHub subcommand: " << subcommand << std::endl;
            return 1;
        }
    }
    else if (command == "help") {
        printUsage();
        return 0;
    }
    else {
        std::cerr << "Unknown command: " << command << std::endl;
        printUsage();
        return 1;
    }
    
    return 0;
}
// To run the program, execute the compiled binary with any command line arguments you wish to test.
// Example usage:
// ./main arg1 arg2 arg3
// This will output:
// Command line arguments received:
// arg1
// arg2
// arg3
// If no arguments are provided, it will output:
// No command line arguments received.
// Note: Ensure that your development environment is set up to use C++17 features.
// This code is a simple demonstration of handling command line arguments in C++.
// It is designed to be straightforward and easy to understand for beginners in C++ programming.
// The program can be extended to perform more complex operations based on the command line arguments provided.                                 