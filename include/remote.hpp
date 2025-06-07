#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include <unordered_map>

/**
 * @file remote.hpp
 * @brief Remote repository management for Mimirion VCS
 * @author Mimirion Team
 * @date June 4, 2025
 * 
 * This file contains the interface and classes for managing remote repositories
 * in Mimirion. It defines the base RemoteProvider interface for different
 * remote hosting services (GitHub, GitLab, etc.) and the RemoteManager class
 * for managing multiple remotes within a repository.
 */

namespace mimirion {

namespace fs = std::filesystem;

/**
 * @class RemoteProvider
 * @brief Abstract base class for remote repository interactions
 * 
 * This abstract class defines the interface for interacting with remote
 * repositories. Different providers (GitHub, GitLab, etc.) can implement
 * this interface to provide specific functionality for their APIs.
 * 
 * The interface supports essential operations like push, pull, clone,
 * and repository creation needed for a VCS to work with remote repositories.
 */
class RemoteProvider {
public:
    /**
     * @brief Virtual destructor
     * 
     * Ensures proper cleanup of derived classes.
     */
    virtual ~RemoteProvider() = default;
    
    /**
     * @brief Push changes to remote repository
     * @param localDir Local repository directory
     * @param remoteName Remote name
     * @param remoteUrl Remote URL
     * @param branch Branch to push
     * @return true if successful, false otherwise
     */
    virtual bool push(const fs::path& localDir, const std::string& remoteName,
                   const std::string& remoteUrl, const std::string& branch) = 0;
    
    /**
     * @brief Pull changes from remote repository
     * @param localDir Local repository directory
     * @param remoteName Remote name
     * @param remoteUrl Remote URL
     * @param branch Branch to pull
     * @return true if successful, false otherwise
     */
    virtual bool pull(const fs::path& localDir, const std::string& remoteName,
                   const std::string& remoteUrl, const std::string& branch) = 0;
    
    /**
     * @brief Clone a remote repository
     * @param remoteUrl Remote URL
     * @param localDir Local directory to clone to
     * @return true if successful, false otherwise
     */
    virtual bool clone(const std::string& remoteUrl, const fs::path& localDir) = 0;
    
    /**
     * @brief Test if the remote is accessible
     * @param remoteUrl Remote URL
     * @return true if accessible, false otherwise
     */
    virtual bool testConnection(const std::string& remoteUrl) = 0;
};

/**
 * @brief Class for managing remote repositories
 */
class RemoteManager {
public:
    RemoteManager(const fs::path& repoPath, const fs::path& mimirionDir);
    
    /**
     * @brief Add a remote repository
     * @param name Remote name
     * @param url Remote URL
     * @return true if successful, false otherwise
     */
    bool addRemote(const std::string& name, const std::string& url);
    
    /**
     * @brief Remove a remote repository
     * @param name Remote name
     * @return true if successful, false otherwise
     */
    bool removeRemote(const std::string& name);
    
    /**
     * @brief Get all remote repositories
     * @return Map of remote names to URLs
     */
    std::unordered_map<std::string, std::string> getRemotes() const;
    
    /**
     * @brief Push to a remote repository
     * @param name Remote name
     * @param branch Branch to push
     * @return true if successful, false otherwise
     */
    bool push(const std::string& name, const std::string& branch);
    
    /**
     * @brief Pull from a remote repository
     * @param name Remote name
     * @param branch Branch to pull
     * @return true if successful, false otherwise
     */
    bool pull(const std::string& name, const std::string& branch);
    
    /**
     * @brief Save the remote configuration to disk
     * @return true if successful, false otherwise
     */
    bool saveState() const;
    
    /**
     * @brief Load the remote configuration from disk
     * @return true if successful, false otherwise
     */
    bool loadState();

private:
    fs::path repositoryPath;
    fs::path mimirionDir;
    std::unordered_map<std::string, std::string> remotes;
    std::shared_ptr<RemoteProvider> provider;
};

} // namespace mimirion
