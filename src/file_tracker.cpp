#include "../include/file_tracker.hpp"
#include "../include/utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>

namespace mimirion {

FileTracker::FileTracker(const fs::path& repoPath, const fs::path& mimirDir)
    : repositoryPath(repoPath), mimirionDir(mimirDir) {
}

void FileTracker::updateStatus() {
    // Update the status of all files in the repository
    std::unordered_map<std::string, FileInfo> oldFiles = files;
    files.clear();
    
    // Walk through repository and collect files
    for (const auto& entry : fs::recursive_directory_iterator(repositoryPath)) {
        // Skip .mimirion directory
        if (entry.path().string().find(mimirionDir.string()) == 0) {
            continue;
        }
        
        // Skip directories
        if (!entry.is_regular_file()) {
            continue;
        }
        
        // Skip ignored files
        if (isIgnored(entry.path())) {
            continue;
        }
        
        // Get relative path to the repository
        std::string relativePath = fs::relative(entry.path(), repositoryPath).string();
        
        // Create or update file info
        FileInfo fileInfo;
        fileInfo.path = relativePath;
        fileInfo.hash = calculateFileHash(entry.path());
        
        // Check if file was previously tracked
        auto it = oldFiles.find(relativePath);
        if (it != oldFiles.end()) {
            fileInfo.lastCommitHash = it->second.lastCommitHash;
            
            // Determine status
            if (fileInfo.hash != fileInfo.lastCommitHash) {
                fileInfo.status = FileStatus::MODIFIED;
            } else {
                fileInfo.status = FileStatus::COMMITTED;
            }
        } else {
            fileInfo.lastCommitHash = "";
            fileInfo.status = FileStatus::UNTRACKED;
        }
        
        files[relativePath] = fileInfo;
    }
    
    // Check for deleted files
    for (const auto& oldFile : oldFiles) {
        if (files.find(oldFile.first) == files.end()) {
            FileInfo fileInfo = oldFile.second;
            fileInfo.status = FileStatus::DELETED;
            files[oldFile.first] = fileInfo;
        }
    }
}

std::vector<FileInfo> FileTracker::getFiles() const {
    std::vector<FileInfo> result;
    result.reserve(files.size());
    
    for (const auto& file : files) {
        result.push_back(file.second);
    }
    
    // Sort by path
    std::sort(result.begin(), result.end(), 
              [](const FileInfo& a, const FileInfo& b) {
                  return a.path < b.path;
              });
    
    return result;
}

bool FileTracker::stageFile(const std::string& path) {
    fs::path fullPath = repositoryPath / path;
    
    // Check if file exists
    if (!fs::exists(fullPath)) {
        std::cerr << "File does not exist: " << path << std::endl;
        return false;
    }
    
    // Get relative path
    std::string relativePath = fs::relative(fullPath, repositoryPath).string();
    
    // Calculate hash
    std::string hash = calculateFileHash(fullPath);
    
    // Create or update file info
    auto it = files.find(relativePath);
    if (it != files.end()) {
        it->second.hash = hash;
        it->second.status = FileStatus::STAGED;
    } else {
        FileInfo fileInfo;
        fileInfo.path = relativePath;
        fileInfo.hash = hash;
        fileInfo.lastCommitHash = "";
        fileInfo.status = FileStatus::STAGED;
        files[relativePath] = fileInfo;
    }
    
    return saveState();
}

bool FileTracker::unstageFile(const std::string& path) {
    // Get relative path
    std::string relativePath = path;
    
    // Check if file is staged
    auto it = files.find(relativePath);
    if (it == files.end() || it->second.status != FileStatus::STAGED) {
        std::cerr << "File is not staged: " << path << std::endl;
        return false;
    }
    
    // Revert to previous status
    if (it->second.lastCommitHash.empty()) {
        it->second.status = FileStatus::UNTRACKED;
    } else {
        fs::path fullPath = repositoryPath / path;
        std::string currentHash = calculateFileHash(fullPath);
        
        if (currentHash == it->second.lastCommitHash) {
            it->second.status = FileStatus::COMMITTED;
        } else {
            it->second.status = FileStatus::MODIFIED;
        }
    }
    
    return saveState();
}

std::vector<FileInfo> FileTracker::getStagedFiles() const {
    std::vector<FileInfo> result;
    
    for (const auto& file : files) {
        if (file.second.status == FileStatus::STAGED) {
            result.push_back(file.second);
        }
    }
    
    // Sort by path
    std::sort(result.begin(), result.end(), 
              [](const FileInfo& a, const FileInfo& b) {
                  return a.path < b.path;
              });
    
    return result;
}

bool FileTracker::saveState() const {
    // Create index file
    std::ofstream indexFile(mimirionDir / "index");
    if (!indexFile) {
        std::cerr << "Failed to save index file" << std::endl;
        return false;
    }
    
    // Write file information
    for (const auto& file : files) {
        indexFile << file.second.path << "\t"
                 << file.second.hash << "\t"
                 << file.second.lastCommitHash << "\t"
                 << static_cast<int>(file.second.status) << std::endl;
    }
    
    indexFile.close();
    return true;
}

bool FileTracker::loadState() {
    // Clear current files
    files.clear();
    
    // Open index file
    std::ifstream indexFile(mimirionDir / "index");
    if (!indexFile) {
        // Index file doesn't exist yet, that's ok
        return true;
    }
    
    // Read file information
    std::string line;
    while (std::getline(indexFile, line)) {
        std::istringstream iss(line);
        std::string path, hash, lastCommitHash;
        int status;
        
        if (std::getline(iss, path, '\t') && 
            std::getline(iss, hash, '\t') && 
            std::getline(iss, lastCommitHash, '\t') && 
            (iss >> status)) {
            
            FileInfo fileInfo;
            fileInfo.path = path;
            fileInfo.hash = hash;
            fileInfo.lastCommitHash = lastCommitHash;
            fileInfo.status = static_cast<FileStatus>(status);
            
            files[path] = fileInfo;
        }
    }
    
    indexFile.close();
    return true;
}

std::string FileTracker::calculateFileHash(const fs::path& filePath) const {
    // Use the utility function to calculate SHA-256 hash
    return utils::sha256File(filePath);
}

void FileTracker::updateFileStatus(FileInfo& file) {
    fs::path fullPath = repositoryPath / file.path;
    
    if (!fs::exists(fullPath)) {
        file.status = FileStatus::DELETED;
        return;
    }
    
    std::string currentHash = calculateFileHash(fullPath);
    
    if (file.lastCommitHash.empty()) {
        file.status = FileStatus::UNTRACKED;
    } else if (currentHash == file.lastCommitHash) {
        file.status = FileStatus::COMMITTED;
    } else {
        file.status = FileStatus::MODIFIED;
    }
}

bool FileTracker::isIgnored(const fs::path& path) const {
    // Check for .mimirionignore file
    fs::path ignoreFile = repositoryPath / ".mimirionignore";
    if (!fs::exists(ignoreFile)) {
        // No ignore file, only ignore .mimirion directory
        return path.string().find(mimirionDir.string()) == 0;
    }
    
    // TODO: Implement more sophisticated ignore pattern matching
    
    // For now, just ignore .mimirion directory
    return path.string().find(mimirionDir.string()) == 0;
}

} // namespace mimirion
