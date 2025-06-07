#include "../include/commit.hpp"
#include "../include/utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace mimirion {

CommitManager::CommitManager(const fs::path& repoPath, const fs::path& mimirDir)
    : repositoryPath(repoPath), mimirionDir(mimirDir), currentHead("") {
}

std::string CommitManager::createCommit(const std::string& message, 
                                     const std::vector<std::string>& stagedFiles) {
    // Check if there are any files to commit
    if (stagedFiles.empty()) {
        std::cerr << "No files staged for commit" << std::endl;
        return "";
    }
    
    // Create new commit
    CommitInfo commit;
    // Strip trailing newlines from message to keep it consistent
    std::string cleanMessage = message;
    while (!cleanMessage.empty() && (cleanMessage.back() == '\n' || cleanMessage.back() == '\r')) {
        cleanMessage.pop_back();
    }
    commit.message = cleanMessage;
    commit.author = utils::getUserName();
    commit.email = utils::getUserEmail();
    commit.timestamp = std::chrono::system_clock::now();
    
    // Add parent commit if there is a HEAD
    if (!currentHead.empty()) {
        commit.parentHashes.push_back(currentHead);
    }
    
    // Add file information
    for (const auto& file : stagedFiles) {
        // TODO: Save file content to objects directory
        commit.fileHashes[file] = "dummy-file-hash"; // Placeholder
    }
    
    // Generate commit hash
    commit.hash = generateCommitHash(commit);
    
    // Save commit object
    if (!saveCommitObject(commit)) {
        return "";
    }
    
    // Update HEAD
    currentHead = commit.hash;
    std::ofstream headFile(mimirionDir / "refs" / "heads" / "master");
    if (!headFile) {
        std::cerr << "Failed to update HEAD" << std::endl;
        return "";
    }
    headFile << commit.hash << std::endl;
    headFile.close();
    
    // Save state
    saveState();
    
    return commit.hash;
}

CommitInfo* CommitManager::getCommit(const std::string& hash) {
    // Check if commit is already loaded
    auto it = commits.find(hash);
    if (it != commits.end()) {
        return &(it->second);
    }
    
    // Load commit from disk
    CommitInfo commit = loadCommitObject(hash);
    if (commit.hash.empty()) {
        return nullptr;
    }
    
    // Add to cache
    commits[hash] = commit;
    return &(commits[hash]);
}

CommitInfo* CommitManager::getHeadCommit() {
    if (currentHead.empty()) {
        return nullptr;
    }
    
    return getCommit(currentHead);
}

std::vector<CommitInfo> CommitManager::getHistory(size_t maxCount) const {
    std::vector<CommitInfo> history;
    
    // Start from HEAD
    std::string current = currentHead;
    size_t count = 0;
    
    while (!current.empty() && (maxCount == 0 || count < maxCount)) {
        // Load commit
        CommitInfo commit = loadCommitObject(current);
        if (commit.hash.empty()) {
            break;
        }
        
        // Add to history
        history.push_back(commit);
        count++;
        
        // Move to parent
        if (commit.parentHashes.empty()) {
            break;
        }
        
        current = commit.parentHashes[0];
    }
    
    return history;
}

bool CommitManager::saveState() const {
    // Create config directory if it doesn't exist
    fs::create_directories(mimirionDir / "config");
    
    // Save HEAD
    std::ofstream headRefFile(mimirionDir / "HEAD");
    if (!headRefFile) {
        std::cerr << "Failed to save HEAD reference" << std::endl;
        return false;
    }
    
    headRefFile << "ref: refs/heads/master" << std::endl;
    headRefFile.close();
    
    return true;
}

bool CommitManager::loadState() {
    // Read current HEAD from refs
    std::ifstream headFile(mimirionDir / "refs" / "heads" / "master");
    if (headFile) {
        std::getline(headFile, currentHead);
        headFile.close();
    } else {
        currentHead = "";
    }
    
    return true;
}

std::string CommitManager::generateCommitHash(const CommitInfo& commit) const {
    // Create a string representation of the commit
    std::stringstream ss;
    ss << "tree " << "dummy-tree-hash" << "\n"; // Placeholder for tree hash
    
    // Add parent commits
    for (const auto& parent : commit.parentHashes) {
        ss << "parent " << parent << "\n";
    }
    
    // Add author and committer information
    ss << "author " << commit.author << " <" << commit.email << "> "
       << utils::formatTimestamp(commit.timestamp) << "\n";
    ss << "committer " << commit.author << " <" << commit.email << "> "
       << utils::formatTimestamp(commit.timestamp) << "\n";
    
    // Add message
    ss << "\n" << commit.message << "\n";
    
    // Calculate hash
    return utils::sha256(ss.str());
}

bool CommitManager::saveCommitObject(const CommitInfo& commit) const {
    // Create objects directory if it doesn't exist
    fs::path objectsDir = mimirionDir / "objects";
    fs::create_directories(objectsDir);
    
    // Create subdirectory based on first 2 characters of hash
    std::string prefix = commit.hash.substr(0, 2);
    std::string suffix = commit.hash.substr(2);
    fs::path commitDir = objectsDir / prefix;
    fs::create_directories(commitDir);
    
    // Create commit file
    std::ofstream commitFile(commitDir / suffix);
    if (!commitFile) {
        std::cerr << "Failed to save commit object" << std::endl;
        return false;
    }
    
    // Write commit information
    commitFile << "commit " << commit.hash << "\n";
    
    // Write parent commits
    for (const auto& parent : commit.parentHashes) {
        commitFile << "parent " << parent << "\n";
    }
    
    // Write author and committer information
    commitFile << "author " << commit.author << " <" << commit.email << "> "
             << utils::formatTimestamp(commit.timestamp) << "\n";
    commitFile << "committer " << commit.author << " <" << commit.email << "> "
             << utils::formatTimestamp(commit.timestamp) << "\n";
    
    // Write message
    commitFile << "\n" << commit.message << "\n";
    
    // Write file hashes
    commitFile << "\nfiles:\n";
    for (const auto& file : commit.fileHashes) {
        commitFile << file.first << "\t" << file.second << "\n";
    }
    
    commitFile.close();
    return true;
}

CommitInfo CommitManager::loadCommitObject(const std::string& hash) const {
    // Create empty commit
    CommitInfo commit;
    
    // Check if hash is valid
    if (hash.length() < 2) {
        return commit;
    }
    
    // Get object path
    std::string prefix = hash.substr(0, 2);
    std::string suffix = hash.substr(2);
    fs::path commitPath = mimirionDir / "objects" / prefix / suffix;
    
    // Check if file exists
    if (!fs::exists(commitPath)) {
        return commit;
    }
    
    // Open commit file
    std::ifstream commitFile(commitPath);
    if (!commitFile) {
        return commit;
    }
    
    // Read commit information
    std::string line;
    
    // Read type and hash
    if (!std::getline(commitFile, line)) {
        return commit;
    }
    
    // Verify it's a commit object
    if (line.substr(0, 7) != "commit ") {
        return commit;
    }
    
    // Set hash
    commit.hash = hash;
    
    // Read parent commits
    while (std::getline(commitFile, line) && !line.empty()) {
        if (line.substr(0, 7) == "parent ") {
            commit.parentHashes.push_back(line.substr(7));
        } else if (line.substr(0, 7) == "author ") {
            // Parse author information
            // Format: "author Name <email> timestamp"
            size_t emailStart = line.find('<');
            size_t emailEnd = line.find('>');
            if (emailStart != std::string::npos && emailEnd != std::string::npos) {
                commit.author = line.substr(7, emailStart - 8);
                commit.email = line.substr(emailStart + 1, emailEnd - emailStart - 1);
                // TODO: Parse timestamp
            }
        }
        // Skip other headers
    }
    
    // Read message
    std::stringstream messageStream;
    while (std::getline(commitFile, line) && line != "files:") {
        messageStream << line << "\n";
    }
    std::string fullMessage = messageStream.str();
    // Strip trailing newlines for consistency
    while (!fullMessage.empty() && (fullMessage.back() == '\n' || fullMessage.back() == '\r')) {
        fullMessage.pop_back();
    }
    commit.message = fullMessage;
    
    // Read file hashes if available
    if (line == "files:") {
        while (std::getline(commitFile, line)) {
            size_t tabPos = line.find('\t');
            if (tabPos != std::string::npos) {
                std::string path = line.substr(0, tabPos);
                std::string fileHash = line.substr(tabPos + 1);
                commit.fileHashes[path] = fileHash;
            }
        }
    }
    
    commitFile.close();
    return commit;
}

} // namespace mimirion
