#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

/**
 * @file file_tracker.hpp
 * @brief File tracking and status management for Mimirion VCS
 * @author Mimirion Team
 * @date June 2025
 * 
 * This file contains the definitions for file tracking related structures
 * and the FileTracker class which manages the status of files in a repository.
 */

namespace mimirion {

namespace fs = std::filesystem;

/**
 * @enum FileStatus
 * @brief Enumeration of possible file statuses in the repository
 */
enum class FileStatus {
    UNTRACKED,    /**< File exists but is not being tracked by the VCS */
    MODIFIED,     /**< File has been modified since last commit */
    STAGED,       /**< File has been staged for the next commit */
    COMMITTED,    /**< File is committed and unchanged */
    DELETED       /**< File was tracked but has been deleted from the filesystem */
};

/**
 * @struct FileInfo
 * @brief Structure containing information about a tracked file
 * 
 * This structure holds all relevant metadata about a file being tracked
 * by the version control system, including its path, content hashes,
 * and current status.
 */
struct FileInfo {
    std::string path;          /**< Relative path to the file from repository root */
    std::string hash;          /**< Hash of the file's current content */
    std::string lastCommitHash; /**< Hash of the file's content at last commit */
    FileStatus status;         /**< Current status of the file */
};

/**
 * @brief Class responsible for tracking files and their states
 */
class FileTracker {
public:
    FileTracker(const fs::path& repoPath, const fs::path& mimirionDir);
    
    /**
     * @brief Update the status of all tracked and untracked files
     */
    void updateStatus();
    
    /**
     * @brief Get a list of all files and their statuses
     * @return Vector of FileInfo objects
     */
    std::vector<FileInfo> getFiles() const;
    
    /**
     * @brief Stage a file for commit
     * @param path Path to the file
     * @return true if successful, false otherwise
     */
    bool stageFile(const std::string& path);
    
    /**
     * @brief Unstage a file
     * @param path Path to the file
     * @return true if successful, false otherwise
     */
    bool unstageFile(const std::string& path);
    
    /**
     * @brief Get a list of staged files
     * @return Vector of FileInfo objects
     */
    std::vector<FileInfo> getStagedFiles() const;
    
    /**
     * @brief Save the current state to disk
     * @return true if successful, false otherwise
     */
    bool saveState() const;
    
    /**
     * @brief Load the state from disk
     * @return true if successful, false otherwise
     */
    bool loadState();

private:
    fs::path repositoryPath;
    fs::path mimirionDir;
    std::unordered_map<std::string, FileInfo> files;
    
    std::string calculateFileHash(const fs::path& filePath) const;
    void updateFileStatus(FileInfo& file);
    bool isIgnored(const fs::path& path) const;
};

} // namespace mimirion
