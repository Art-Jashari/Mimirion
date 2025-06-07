# GitHub Integration Guide for Mimirion VCS

This guide explains how to use Mimirion VCS with GitHub repositories.

## Prerequisites

Before using Mimirion with GitHub, you'll need:

1. A GitHub account
2. A Personal Access Token (PAT) with appropriate permissions
3. Mimirion VCS compiled with GitHub integration support

## Creating a GitHub Personal Access Token

1. Log in to your GitHub account
2. Navigate to Settings > Developer settings > Personal access tokens
3. Click "Generate new token"
4. Select the following scopes:
   - `repo` (Full control of private repositories)
   - `workflow` (If you plan to use GitHub Actions)
5. Click "Generate token"
6. Copy the token – you won't be able to see it again!

## Setting up GitHub Integration with Mimirion

### Option 1: Using the API directly

```cpp
#include <iostream>
#include "../include/repository.hpp"

int main() {
    // Initialize or load repository
    mimirion::Repository repo;
    repo.load("/path/to/your/repo");
    
    // Set GitHub credentials
    repo.setGitHubCredentials("your-username", "your-personal-access-token");
    
    // Add a GitHub remote
    repo.addRemote("origin", "https://github.com/your-username/your-repo.git");
    
    // Now you can push or pull
    repo.push("origin", "master");
}
```

### Option 2: Using a credentials file

1. Create a credentials file with your GitHub token:
   ```
   username=your-github-username
   token=your-personal-access-token
   ```

2. Use it in your code:
   ```cpp
   repo.setGitHubCredentialsFromFile("/path/to/github_credentials.txt");
   ```

### Option 3: Using the GitHub Example Tool

We provide a ready-to-use tool for testing GitHub integration:

```bash
# Build the GitHub example tool
cd /path/to/Mimirion
mkdir -p build && cd build
cmake ..
make github_example

# Run it
./bin/github_example your-github-username your-personal-access-token /path/to/your/repo
```

## Common GitHub Operations

### Pushing to GitHub

```cpp
// First, make sure you've set up credentials and remotes
repo.add("file.txt");
repo.commit("Add new file");
repo.push("origin", "master");
```

### Pulling from GitHub

```cpp
repo.pull("origin", "master");
```

### Working with branches

```cpp
repo.createBranch("feature");
repo.checkout("feature");
// Make your changes...
repo.add("changed_file.txt");
repo.commit("Update file in feature branch");
repo.push("origin", "feature");
```

## Troubleshooting

- **Authentication Errors**: Make sure your Personal Access Token has the correct permissions.
- **Push Failures**: Check if the remote repository exists and you have write access.
- **Network Issues**: Verify your internet connection and firewall settings.

For more advanced GitHub API operations, you may need to extend the `GitHubProvider` class with additional functionality.
