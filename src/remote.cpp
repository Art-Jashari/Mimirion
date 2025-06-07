#include "../include/remote.hpp"
#include "../include/github_api.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace mimirion {

RemoteManager::RemoteManager(const fs::path& repoPath, const fs::path& mimirDir)
    : repositoryPath(repoPath), mimirionDir(mimirDir) {
    // Initialize provider with GitHub for now
    provider = std::make_shared<GitHubProvider>();
    
    // Load state
    loadState();
}

bool RemoteManager::addRemote(const std::string& name, const std::string& url) {
    // Add remote to map
    remotes[name] = url;
    
    // Save state
    return saveState();
}

bool RemoteManager::removeRemote(const std::string& name) {
    // Check if remote exists
    auto it = remotes.find(name);
    if (it == remotes.end()) {
        std::cerr << "Remote does not exist: " << name << std::endl;
        return false;
    }
    
    // Remove remote
    remotes.erase(it);
    
    // Save state
    return saveState();
}

std::unordered_map<std::string, std::string> RemoteManager::getRemotes() const {
    return remotes;
}

bool RemoteManager::push(const std::string& name, const std::string& branch) {
    // Check if remote exists
    auto it = remotes.find(name);
    if (it == remotes.end()) {
        std::cerr << "Remote does not exist: " << name << std::endl;
        return false;
    }
    
    // Push to remote
    return provider->push(repositoryPath, name, it->second, branch);
}

bool RemoteManager::pull(const std::string& name, const std::string& branch) {
    // Check if remote exists
    auto it = remotes.find(name);
    if (it == remotes.end()) {
        std::cerr << "Remote does not exist: " << name << std::endl;
        return false;
    }
    
    // Pull from remote
    return provider->pull(repositoryPath, name, it->second, branch);
}

bool RemoteManager::saveState() const {
    // Create config directory if it doesn't exist
    fs::create_directories(mimirionDir / "config");
    
    // Save remotes
    std::ofstream remotesFile(mimirionDir / "config" / "remotes");
    if (!remotesFile) {
        std::cerr << "Failed to save remotes configuration" << std::endl;
        return false;
    }
    
    for (const auto& remote : remotes) {
        remotesFile << remote.first << " " << remote.second << std::endl;
    }
    
    remotesFile.close();
    return true;
}

bool RemoteManager::loadState() {
    // Clear current remotes
    remotes.clear();
    
    // Load remotes
    std::ifstream remotesFile(mimirionDir / "config" / "remotes");
    if (!remotesFile) {
        // File doesn't exist yet, that's ok
        return true;
    }
    
    std::string line;
    while (std::getline(remotesFile, line)) {
        size_t spacePos = line.find(' ');
        if (spacePos != std::string::npos) {
            std::string name = line.substr(0, spacePos);
            std::string url = line.substr(spacePos + 1);
            remotes[name] = url;
        }
    }
    
    remotesFile.close();
    return true;
}

} // namespace mimirion
