#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include "github_api.hpp"

/**
 * @file repository.hpp
 * @brief Core repository management for the Mimirion VCS
 * @author Mimirion Team
 * @date June 2025
 * 
 * This file contains the definitions for the Repository class,
 * which handles core operations for version control such as:
 * initialization, status checking, file staging, committing,
 * branching, and remote operations.
 */

namespace mimirion {

namespace fs = std::filesystem;

/**
 * @class Repository
 * @brief Core class representing a Mimirion version control repository
 * 
 * The Repository class is the main entry point for interacting with
 * a Mimirion version control repository. It provides methods for
 * all core VCS operations including initialization, status checking,
 * staging files, committing changes, branch management, and remote
 * integration.
 */
class Repository {
public:
    /**
     * @brief Constructor for the Repository class
     * 
     * Creates a new Repository instance without initializing or loading
     * an actual repository. Use init() or load() to initialize or load
     * an existing repository after construction.
     */
    Repository();
    
    /**
     * @brief Initialize a new repository in the given directory
     * 
     * This method creates a new Mimirion repository in the specified directory.
     * It creates the necessary directory structure and initial files for the repository.
     * If the directory does not exist, it will be created.
     * 
     * The initialized repository will have:
     * - A .mimirion directory for metadata
     * - A master branch as the default branch
     * - Standard directory structure for objects and references
     * 
     * @param path Directory path to initialize
     * @return true if initialization was successful, false if an error occurred
     * @throws None, but outputs errors to stderr
     */
    bool init(const std::string& path);
    
    /**
     * @brief Load an existing repository from the given path
     * 
     * This method attempts to load an existing Mimirion repository from
     * the specified path. If the path itself isn't a repository, it will
     * search up the directory tree for a valid repository.
     * 
     * Once loaded, the repository's state is initialized from the disk,
     * including branch information, remotes, and tracking status.
     * 
     * @param path Path to the repository or a subdirectory within a repository
     * @return true if repository was successfully loaded, false if no valid repository was found
     * @throws None, but errors are output to stderr
     * @see isValidRepository()
     */
    bool load(const std::string& path);
    
    /**
     * @brief Get the current status of files in the repository
     * 
     * Generates a detailed status message showing:
     * - Current branch
     * - Staged changes ready for commit
     * - Modified files not staged for commit
     * - Untracked files
     * 
     * @return Formatted status message string
     * @throws None, but errors are output to stderr
     */
    std::string status() const;
    
    /**
     * @brief Add a file or directory to tracking
     * 
     * Stages a file or directory for the next commit. If a directory is specified,
     * all files in the directory will be recursively added.
     * 
     * This operation:
     * 1. Calculates the hash of the file's contents
     * 2. Stores the file in the object database
     * 3. Updates the index to reflect the staged state
     * 
     * @param path Path to the file or directory to add
     * @return true if successfully added, false if an error occurred
     * @throws None, but errors are output to stderr
     */
    bool add(const std::string& path);
    
    /**
     * @brief Remove a file or directory from tracking
     * @param path Path to remove
     * @return true if successful, false otherwise
     */
    bool remove(const std::string& path);
    
    /**
     * @brief Create a commit with the given message
     * @param message Commit message
     * @return Commit hash if successful, empty string otherwise
     */
    std::string commit(const std::string& message);
    
    /**
     * @brief Create a new branch
     * @param name Name of the branch
     * @return true if successful, false otherwise
     */
    bool createBranch(const std::string& name);
    
    /**
     * @brief Switch to a branch
     * @param name Branch name
     * @return true if successful, false otherwise
     */
    bool checkout(const std::string& name);
    
    /**
     * @brief Push changes to a remote repository
     * @param remote Remote name
     * @param branch Branch name
     * @return true if successful, false otherwise
     */
    bool push(const std::string& remote = "origin", const std::string& branch = "");
    
    /**
     * @brief Pull changes from a remote repository
     * @param remote Remote name
     * @param branch Branch name
     * @return true if successful, false otherwise
     */
    bool pull(const std::string& remote = "origin", const std::string& branch = "");
    
    /**
     * @brief Add a remote repository
     * @param name Remote name
     * @param url Remote URL
     * @return true if successful, false otherwise
     */
    bool addRemote(const std::string& name, const std::string& url);
    
    /**
     * @brief Set GitHub credentials for API operations
     * @param username GitHub username
     * @param token GitHub personal access token
     * @return true if successful, false otherwise
     */
    bool setGitHubCredentials(const std::string& username, const std::string& token);
    
    /**
     * @brief Set GitHub credentials from a token file
     * @param tokenFilePath Path to the token file
     * @return true if successful, false otherwise
     */
    bool setGitHubCredentialsFromFile(const fs::path& tokenFilePath);

private:
    /** @brief Absolute path to the repository's root directory */
    fs::path repositoryPath;
    
    /** @brief Absolute path to the repository's metadata directory (.mimirion) */
    fs::path mimirionDir;
    
    /** @brief Name of the currently checked out branch */
    std::string currentBranch;
    
    /** @brief Map of remote names to their URLs */
    std::unordered_map<std::string, std::string> remotes;
    
    /** @brief List of staged files awaiting commit */
    std::vector<std::string> stagedFiles;
    
    /** @brief GitHub provider for remote operations */
    std::unique_ptr<GitHubProvider> githubProvider;
    
    /**
     * @brief Validates that the current paths point to a valid repository
     * 
     * Checks for the existence and structure of the .mimirion directory
     * and essential repository files.
     * 
     * @return true if the repository is valid, false otherwise
     */
    bool isValidRepository() const;
    
    /**
     * @brief Save the repository state to disk
     * 
     * Saves the current state of the repository, including branch information
     * and remote settings to the .mimirion directory.
     * 
     * @return true if state was saved successfully, false otherwise
     */
    bool saveState() const;
    
    /**
     * @brief Load the repository state from disk
     * 
     * Loads the repository state from the .mimirion directory, including
     * branch information and remote settings.
     * 
     * @return true if state was loaded successfully, false otherwise
     */
    bool loadState();
};

} // namespace mimirion
