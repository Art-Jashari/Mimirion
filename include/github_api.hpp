#pragma once

#include "remote.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <curl/curl.h>

/**
 * @file github_api.hpp
 * @brief GitHub API integration for Mimirion VCS
 * @author Mimirion Team
 * @date June 4, 2025
 * 
 * This file contains the definitions for GitHub API integration,
 * implementing the RemoteProvider interface for communication with
 * GitHub repositories. It provides functionality for authentication,
 * pushing, pulling, and repository management.
 */

namespace mimirion {

namespace fs = std::filesystem;

/**
 * @struct GitHubCredentials
 * @brief Structure to store GitHub authentication credentials
 * 
 * This structure stores GitHub user credentials needed for API authentication,
 * including username, personal access token, and the path to the token file.
 */
struct GitHubCredentials {
    std::string username;   /**< GitHub account username */
    std::string token;      /**< GitHub Personal Access Token for API authentication */
    std::string tokenFile;  /**< Path to file storing the token for persistence */
};

/**
 * @class GitHubProvider
 * @brief GitHub API implementation of the RemoteProvider interface
 * 
 * The GitHubProvider class implements the RemoteProvider interface to
 * communicate with GitHub repositories. It handles authentication,
 * pushing, pulling, and repository management through the GitHub API.
 * 
 * This class uses libcurl for HTTP requests and manages GitHub API
 * authentication with personal access tokens.
 */
class GitHubProvider : public RemoteProvider {
public:
    /**
     * @brief Constructor for GitHubProvider
     * 
     * Initializes the libcurl library and creates a cURL handle for
     * making HTTP requests to the GitHub API.
     */
    GitHubProvider();
    
    /**
     * @brief Destructor for GitHubProvider
     * 
     * Cleans up the cURL handle and performs libcurl global cleanup.
     */
    ~GitHubProvider() override;
    
    /**
     * @brief Set GitHub credentials
     * @param username GitHub username
     * @param token GitHub Personal Access Token
     */
    void setCredentials(const std::string& username, const std::string& token);
    
    /**
     * @brief Set GitHub credentials from a file
     * @param path Path to credentials file
     * @return true if successful, false otherwise
     */
    bool setCredentialsFromFile(const fs::path& path);
    
    /**
     * @brief Save GitHub credentials to a file
     * @param path Path to credentials file
     * @return true if successful, false otherwise
     */
    bool saveCredentialsToFile(const fs::path& path) const;
    
    /**
     * @brief Push changes to GitHub repository
     * @param localDir Local repository directory
     * @param remoteName Remote name
     * @param remoteUrl Remote URL
     * @param branch Branch to push
     * @return true if successful, false otherwise
     */
    bool push(const fs::path& localDir, const std::string& remoteName,
           const std::string& remoteUrl, const std::string& branch) override;
    
    /**
     * @brief Pull changes from GitHub repository
     * @param localDir Local repository directory
     * @param remoteName Remote name
     * @param remoteUrl Remote URL
     * @param branch Branch to pull
     * @return true if successful, false otherwise
     */
    bool pull(const fs::path& localDir, const std::string& remoteName,
           const std::string& remoteUrl, const std::string& branch) override;
    
    /**
     * @brief Clone a GitHub repository
     * @param remoteUrl Remote URL
     * @param localDir Local directory to clone to
     * @return true if successful, false otherwise
     */
    bool clone(const std::string& remoteUrl, const fs::path& localDir) override;
    
    /**
     * @brief Test if the GitHub repository is accessible
     * @param remoteUrl Remote URL
     * @return true if accessible, false otherwise
     */
    bool testConnection(const std::string& remoteUrl) override;
    
    /**
     * @brief Create a new GitHub repository
     * @param name Repository name
     * @param description Repository description
     * @param isPrivate Whether the repository is private
     * @return Repository URL if successful, empty string otherwise
     */
    std::string createRepository(const std::string& name, const std::string& description = "", 
                              bool isPrivate = false);
    
    /**
     * @brief Get repositories owned by the authenticated user
     * @return Vector of repository names
     */
    std::vector<std::string> getRepositories();

private:
    GitHubCredentials credentials;
    CURL* curl;
    
    bool executeRequest(const std::string& url, const std::string& method = "GET",
                      const std::string& data = "", std::string* response = nullptr);
    
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    
    std::string getRepoApiUrl(const std::string& remoteUrl) const;
    std::string extractOwnerAndRepo(const std::string& remoteUrl, std::string* owner = nullptr) const;
};

} // namespace mimirion
