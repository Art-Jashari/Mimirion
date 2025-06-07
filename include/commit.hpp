#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <chrono>

/**
 * @file commit.hpp
 * @brief Commit management for the Mimirion VCS
 * @author Mimirion Team
 * @date June 2025
 * 
 * This file contains the definitions for commit-related structures and
 * the CommitManager class which handles creating, storing and retrieving
 * commits in the Mimirion version control system.
 */

namespace mimirion {

namespace fs = std::filesystem;

/**
 * @struct CommitInfo
 * @brief Structure containing all data for a single commit
 *
 * This structure represents all metadata and content references
 * for a single commit in the version control system.
 */
struct CommitInfo {
    std::string hash;               /**< Unique hash identifying the commit */
    std::string message;            /**< Commit message provided by the user */
    std::string author;             /**< Name of the commit author */
    std::string email;              /**< Email of the commit author */
    std::chrono::system_clock::time_point timestamp; /**< Time when the commit was created */
    std::vector<std::string> parentHashes; /**< Hashes of parent commits (multiple for merge commits) */
    std::unordered_map<std::string, std::string> fileHashes; /**< Map of file paths to their content hashes */
};

/**
 * @class CommitManager
 * @brief Class responsible for managing commits in a Mimirion repository
 * 
 * The CommitManager handles all operations related to creating, storing,
 * retrieving, and analyzing commits in a Mimirion repository. It provides
 * methods for commit creation, history traversal, and state management.
 */
class CommitManager {
public:
    /**
     * @brief Constructor for CommitManager
     * 
     * Creates a new CommitManager instance for the specified repository.
     * 
     * @param repoPath Path to the repository root directory
     * @param mimirionDir Path to the repository's .mimirion directory
     */
    CommitManager(const fs::path& repoPath, const fs::path& mimirionDir);
    
    /**
     * @brief Create a new commit with the given message
     * @param message Commit message
     * @param stagedFiles Files staged for commit
     * @return Commit hash if successful, empty string otherwise
     */
    std::string createCommit(const std::string& message, 
                           const std::vector<std::string>& stagedFiles);
    
    /**
     * @brief Get a commit by its hash
     * @param hash Commit hash
     * @return CommitInfo object if found, nullptr otherwise
     */
    CommitInfo* getCommit(const std::string& hash);
    
    /**
     * @brief Get the current HEAD commit
     * @return CommitInfo object if found, nullptr otherwise
     */
    CommitInfo* getHeadCommit();
    
    /**
     * @brief Get the commit history
     * @param maxCount Maximum number of commits to return (0 for all)
     * @return Vector of CommitInfo objects
     */
    std::vector<CommitInfo> getHistory(size_t maxCount = 0) const;
    
    /**
     * @brief Save the commit database to disk
     * @return true if successful, false otherwise
     */
    bool saveState() const;
    
    /**
     * @brief Load the commit database from disk
     * @return true if successful, false otherwise
     */
    bool loadState();

private:
    fs::path repositoryPath;
    fs::path mimirionDir;
    std::string currentHead;
    std::unordered_map<std::string, CommitInfo> commits;
    
    std::string generateCommitHash(const CommitInfo& commit) const;
    bool saveCommitObject(const CommitInfo& commit) const;
    CommitInfo loadCommitObject(const std::string& hash) const;
};

} // namespace mimirion
