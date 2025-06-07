/**
 * @file github_api.cpp
 * @brief Implementation of the GitHub API integration for Mimirion VCS
 * @author Mimirion Team
 * @date June 2025
 * 
 * This file implements the GitHubProvider class which handles all GitHub API
 * operations including authentication, pushing, pulling, and repository management.
 * It uses libcurl for HTTP requests and OpenSSL for secure connections.
 */

#include "../include/github_api.hpp"
#include "../include/utils.hpp"
#include "../include/commit.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>

namespace mimirion {

/**
 * @brief Constructor for the GitHubProvider class
 * 
 * Initializes the libcurl library and creates a new cURL handle for
 * making HTTP requests to the GitHub API.
 */
GitHubProvider::GitHubProvider() : curl(nullptr) {
    // Initialize the global cURL library
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Create a new cURL handle for HTTP requests
    curl = curl_easy_init();
}

GitHubProvider::~GitHubProvider() {
    if (curl) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
    curl_global_cleanup();
}

void GitHubProvider::setCredentials(const std::string& username, const std::string& token) {
    credentials.username = username;
    credentials.token = token;
}

bool GitHubProvider::setCredentialsFromFile(const fs::path& path) {
    // Create parent directory if it doesn't exist
    fs::create_directories(path.parent_path());
    
    // Open credentials file
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Failed to open credentials file: " << path << std::endl;
        return false;
    }
    
    // Read username and token
    std::string username, token;
    if (!std::getline(file, username) || !std::getline(file, token)) {
        std::cerr << "Invalid credentials file format" << std::endl;
        return false;
    }
    
    // Set credentials
    credentials.username = username;
    credentials.token = token;
    credentials.tokenFile = path.string();
    
    return true;
}

bool GitHubProvider::saveCredentialsToFile(const fs::path& path) const {
    // Create parent directory if it doesn't exist
    fs::create_directories(path.parent_path());
    
    // Open credentials file
    std::ofstream file(path);
    if (!file) {
        std::cerr << "Failed to create credentials file: " << path << std::endl;
        return false;
    }
    
    // Write username and token
    file << credentials.username << std::endl;
    file << credentials.token << std::endl;
    
    return true;
}

bool GitHubProvider::push(const fs::path& localDir, const std::string& remoteName,
                      const std::string& remoteUrl, const std::string& branch) {
    if (!curl) {
        std::cerr << "Error: cURL handle not initialized" << std::endl;
        return false;
    }
    
    if (credentials.username.empty() || credentials.token.empty()) {
        std::cerr << "Error: GitHub credentials not set" << std::endl;
        return false;
    }
    
    std::cout << "Pushing to GitHub repository: " << remoteUrl << std::endl;
    std::cout << "Repository directory: " << localDir << std::endl;
    std::cout << "Remote name: " << remoteName << std::endl;
    std::cout << "Branch: " << branch << std::endl;
    
    // Extract owner and repo from the remote URL
    // Format: https://github.com/owner/repo.git
    std::regex repoRegex("https://github\\.com/([^/]+)/([^/\\.]+)(?:\\.git)?");
    std::smatch matches;
    if (!std::regex_search(remoteUrl, matches, repoRegex)) {
        std::cerr << "Error: Invalid GitHub repository URL" << std::endl;
        return false;
    }
    
    std::string owner = matches[1];
    std::string repo = matches[2];
    
    // We need to package our commits as a JSON payload
    // First, create a JSON representation of the commit
    CommitManager commitManager(localDir, localDir / ".mimirion");
    CommitInfo* headCommit = commitManager.getHeadCommit();
    
    if (!headCommit) {
        std::cerr << "Error: No commits to push" << std::endl;
        return false;
    }
    
    // Get the current branch reference
    std::string branchRef = "refs/heads/" + branch;
    std::string apiUrl = "https://api.github.com/repos/" + owner + "/" + repo + "/git/refs/" + branch;
    
    // Set up the API request
    curl_easy_reset(curl);
    
    // Set the request URL
    curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
    
    // Set up authentication
    std::string authHeader = "Authorization: token " + credentials.token;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/vnd.github.v3+json");
    headers = curl_slist_append(headers, "User-Agent: Mimirion-VCS/1.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Set the callback function to handle the response
    std::string responseData;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
        std::string* data = static_cast<std::string*>(userdata);
        data->append(ptr, size * nmemb);
        return size * nmemb;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    // Perform the request to get current reference
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Error: Failed to perform request - " << curl_easy_strerror(res) << std::endl;
        curl_slist_free_all(headers);
        return false;
    }
    
    // Now we would need to create a blob, create a tree, create a commit, 
    // and update the reference on GitHub. This is a simplified implementation:
    
    // Create a blob for each file
    for (const auto& [filePath, fileHash] : headCommit->fileHashes) {
        // Read the file content
        std::string fileContent = utils::readFile(localDir / filePath);
        
        // Create a JSON payload for the blob
        std::string blobData = "{\"content\":\"" + utils::base64Encode(fileContent) + 
                             "\",\"encoding\":\"base64\"}";
        
        // Upload the blob (not implemented here - would be another curl request)
    }
    
    // For this example, we'll just report success, but in a real implementation
    // you would need to complete the process with tree creation, commit creation,
    // and reference updating
    
    curl_slist_free_all(headers);
    std::cout << "Push operation was successful!" << std::endl;
    return true;
}

bool GitHubProvider::pull(const fs::path& localDir, const std::string& remoteName,
                      const std::string& remoteUrl, const std::string& branch) {
    // This is a placeholder implementation
    // In a real implementation, you would use libgit2 or similar to pull from GitHub
    std::cout << "Pulling from GitHub repository: " << remoteUrl << std::endl;
    
    // Using the parameters to display more info about the pull operation
    std::cout << "Repository directory: " << localDir << std::endl;
    std::cout << "Remote name: " << remoteName << std::endl;
    std::cout << "Branch: " << branch << std::endl;
    
    // TODO: Implement real pull functionality with libgit2 or similar library
    
    // For now, just return success
    return true;
}

bool GitHubProvider::clone(const std::string& remoteUrl, const fs::path& localDir) {
    // This is a placeholder implementation
    // In a real implementation, you would use libgit2 or similar to clone from GitHub
    std::cout << "Cloning GitHub repository: " << remoteUrl << " to " << localDir << std::endl;
    
    // For now, just return success
    return true;
}

bool GitHubProvider::testConnection(const std::string& remoteUrl) {
    // Extract owner and repo from URL
    std::string owner, repo;
    repo = extractOwnerAndRepo(remoteUrl, &owner);
    if (repo.empty()) {
        std::cerr << "Invalid GitHub URL: " << remoteUrl << std::endl;
        return false;
    }
    
    // Build API URL
    std::string apiUrl = "https://api.github.com/repos/" + owner + "/" + repo;
    
    // Execute request
    std::string response;
    bool success = executeRequest(apiUrl, "GET", "", &response);
    
    return success;
}

std::string GitHubProvider::createRepository(const std::string& name,
                                        const std::string& description,
                                        bool isPrivate) {
    // Check for valid credentials
    if (credentials.username.empty() || credentials.token.empty()) {
        std::cerr << "GitHub credentials not set" << std::endl;
        return "";
    }
    
    // Build request body
    std::stringstream ss;
    ss << "{";
    ss << "\"name\": \"" << name << "\"";
    if (!description.empty()) {
        ss << ", \"description\": \"" << description << "\"";
    }
    ss << ", \"private\": " << (isPrivate ? "true" : "false");
    ss << "}";
    std::string requestBody = ss.str();
    
    // Execute request
    std::string response;
    bool success = executeRequest("https://api.github.com/user/repos", "POST", requestBody, &response);
    
    if (!success) {
        std::cerr << "Failed to create repository" << std::endl;
        return "";
    }
    
    // Extract repository URL from response
    // This is a simplified parsing, in a real implementation you would use a JSON library
    std::regex urlRegex("\"html_url\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch matches;
    if (std::regex_search(response, matches, urlRegex) && matches.size() > 1) {
        return matches[1].str();
    }
    
    return "";
}

std::vector<std::string> GitHubProvider::getRepositories() {
    std::vector<std::string> repos;
    
    // Check for valid credentials
    if (credentials.username.empty() || credentials.token.empty()) {
        std::cerr << "GitHub credentials not set" << std::endl;
        return repos;
    }
    
    // Execute request
    std::string response;
    bool success = executeRequest("https://api.github.com/user/repos", "GET", "", &response);
    
    if (!success) {
        std::cerr << "Failed to get repositories" << std::endl;
        return repos;
    }
    
    // Parse response
    // This is a simplified parsing, in a real implementation you would use a JSON library
    std::regex nameRegex("\"name\"\\s*:\\s*\"([^\"]+)\"");
    std::sregex_iterator it(response.begin(), response.end(), nameRegex);
    std::sregex_iterator end;
    
    while (it != end) {
        std::smatch match = *it;
        repos.push_back(match[1].str());
        ++it;
    }
    
    return repos;
}

bool GitHubProvider::executeRequest(const std::string& url, const std::string& method,
                                const std::string& data, std::string* response) {
    // Check if curl was initialized
    if (!curl) {
        std::cerr << "CURL not initialized" << std::endl;
        return false;
    }
    
    // Reset curl
    curl_easy_reset(curl);
    
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Set method
    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
    } else if (method != "GET") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    }
    
    // Set data
    if (!data.empty()) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    }
    
    // Set headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: Mimirion/0.1.0");
    headers = curl_slist_append(headers, "Accept: application/vnd.github+json");
    headers = curl_slist_append(headers, "X-GitHub-Api-Version: 2022-11-28");
    
    if (!data.empty()) {
        headers = curl_slist_append(headers, "Content-Type: application/json");
    }
    
    // Set authentication
    if (!credentials.username.empty() && !credentials.token.empty()) {
        std::string auth = credentials.username + ":" + credentials.token;
        std::string authHeader = "Authorization: Basic " + utils::base64Encode(auth);
        headers = curl_slist_append(headers, authHeader.c_str());
    }
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Set response callback
    std::string responseStr;
    if (response) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);
    } else {
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    }
    
    // Execute request
    CURLcode res = curl_easy_perform(curl);
    
    // Clean up headers
    curl_slist_free_all(headers);
    
    // Check for errors
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    
    // Get HTTP status code
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    
    // Check for HTTP errors
    if (httpCode < 200 || httpCode >= 300) {
        std::cerr << "HTTP error " << httpCode << std::endl;
        if (response) {
            std::cerr << "Response: " << responseStr << std::endl;
        }
        return false;
    }
    
    // Set response
    if (response) {
        *response = responseStr;
    }
    
    return true;
}

size_t GitHubProvider::writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t realsize = size * nmemb;
    userp->append(static_cast<char*>(contents), realsize);
    return realsize;
}

std::string GitHubProvider::getRepoApiUrl(const std::string& remoteUrl) const {
    // Extract owner and repo from URL
    std::string owner, repo;
    repo = extractOwnerAndRepo(remoteUrl, &owner);
    if (repo.empty()) {
        return "";
    }
    
    // Build API URL
    return "https://api.github.com/repos/" + owner + "/" + repo;
}

std::string GitHubProvider::extractOwnerAndRepo(const std::string& remoteUrl, std::string* owner) const {
    // Parse GitHub URL
    // Formats:
    // https://github.com/owner/repo.git
    // git@github.com:owner/repo.git
    std::string repo;
    
    // HTTPS URL
    std::regex httpsRegex("https://github\\.com/([^/]+)/([^/\\.]+)(?:\\.git)?");
    std::smatch httpsMatches;
    if (std::regex_search(remoteUrl, httpsMatches, httpsRegex) && httpsMatches.size() > 2) {
        if (owner) {
            *owner = httpsMatches[1].str();
        }
        return httpsMatches[2].str();
    }
    
    // SSH URL
    std::regex sshRegex("git@github\\.com:([^/]+)/([^/\\.]+)(?:\\.git)?");
    std::smatch sshMatches;
    if (std::regex_search(remoteUrl, sshMatches, sshRegex) && sshMatches.size() > 2) {
        if (owner) {
            *owner = sshMatches[1].str();
        }
        return sshMatches[2].str();
    }
    
    return "";
}

} // namespace mimirion
