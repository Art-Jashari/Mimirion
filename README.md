# Mimirion

A custom version control system (VCS) with GitHub integration.

![Mimirion Logo](docs/images/logo.png) <!-- You'll need to create this later -->

## Overview

Mimirion is a simple yet powerful version control system built in C++ that aims to provide functionality similar to Git, with a special focus on GitHub integration. This project demonstrates the core concepts of a VCS while providing a foundation for more advanced features.

## Features

- **Repository Management**: Initialize and manage version-controlled repositories
- **File Tracking**: Track changes to files with automatic status detection
- **Staging Area**: Stage changes before committing them
- **Commit History**: Create commits with detailed messages and track history
- **Branching**: Create, switch, and manage branches for parallel development
- **Remote Integration**: Connect with remote repositories
- **GitHub API**: Seamless integration with GitHub for pushing, pulling, and repository creation
- **Diffing and Patching**: View differences between files and versions

## Architecture

Mimirion follows a modular architecture with clear separation of concerns:

![Architecture Diagram](docs/images/Architecture.png)

*Note: The architecture diagram is generated using PlantUML from the source file in docs/images/architecture.puml*

## Installation

### Prerequisites

To build and run Mimirion, you need:

- C++17 compatible compiler (e.g., GCC 8+, Clang 7+)
- CMake 3.13+
- OpenSSL development libraries
- libcurl development libraries
- zlib development libraries

#### Ubuntu/Debian

```bash
sudo apt-get install build-essential cmake libssl-dev libcurl4-openssl-dev zlib1g-dev
```

#### Manjaro/Arch Linux

```bash
sudo pacman -S base-devel cmake openssl curl zlib
```

#### macOS

```bash
brew install cmake openssl curl zlib
```

#### Windows

Install [MSVC](https://visualstudio.microsoft.com/downloads/), [CMake](https://cmake.org/download/), and [vcpkg](https://github.com/microsoft/vcpkg) for package management:

```bash
vcpkg install openssl curl zlib
```

### Dependencies with vcpkg

Mimirion uses vcpkg for dependency management. The simplest way to build is to use the provided build script:

```bash
# Use the automatic build script (recommended)
./scripts/build_with_vcpkg.sh
```

This script will automatically:
- Check for vcpkg installation or install it if needed
- Configure the project with CMake and vcpkg integration
- Build the project with optimal parallelism

#### Manual Build with vcpkg

If you prefer to build manually:

```bash
# Set the VCPKG_ROOT environment variable if not already set
export VCPKG_ROOT=/path/to/vcpkg

# Build with vcpkg integration
mkdir -p build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake ..
make -j$(nproc)
```

If you don't have vcpkg installed:

```bash
# Install vcpkg
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
export VCPKG_ROOT=$(pwd)/vcpkg

# Build with vcpkg integration
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake ..
make
```

For Windows with MSVC:

```bash
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ..
cmake --build . --config Release
```

## Documentation

Complete API documentation is available in [docs/html/index.html](docs/html/index.html) after building the documentation.

To build the documentation:

```bash
doxygen Doxyfile
```

## Usage

### Basic Operations

#### Initialize a Repository

```bash
mimirion init [path]
```

Example:
```bash
mimirion init my-project
```

#### Check Status

```bash
mimirion status
```

Output:
```
On branch master

Changes to be committed:
  (use "mimirion reset <file>..." to unstage)
        modified:   src/main.cpp

Changes not staged for commit:
  (use "mimirion add <file>..." to update what will be committed)
  (use "mimirion checkout -- <file>..." to discard changes)
        modified:   include/utils.hpp

Untracked files:
  (use "mimirion add <file>..." to include in what will be committed)
        docs/
        tests/
```

#### Add Files to Staging Area

```bash
mimirion add <path>
```

Examples:
```bash
mimirion add src/main.cpp       # Add a specific file
mimirion add include/           # Add all files in a directory
mimirion add .                  # Add all files in the current directory
```

#### Create a Commit

```bash
mimirion commit <message>
```

Example:
```bash
mimirion commit "Add file tracking functionality"
```

#### Work with Branches

Create a branch:
```bash
mimirion branch <name>
```

Switch to a branch:
```bash
mimirion checkout <name>
```

Example workflow:
```bash
mimirion branch feature-login
mimirion checkout feature-login
# Make changes...
mimirion add .
mimirion commit "Implement login feature"
mimirion checkout master
```

### Remote Operations

#### Add a Remote Repository

```bash
mimirion remote add <name> <url>
```

Example:
```bash
mimirion remote add origin https://github.com/username/repository.git
```

#### List Remote Repositories

```bash
mimirion remote list
```

Output:
```
origin  https://github.com/username/repository.git
upstream  https://github.com/original/repository.git
```

#### Push to a Remote Repository

```bash
mimirion push [remote] [branch]
```

Example:
```bash
mimirion push origin master
```

#### Pull from a Remote Repository

```bash
mimirion pull [remote] [branch]
```

Example:
```bash
mimirion pull origin master
```

### GitHub Integration

#### Set GitHub Credentials

```bash
mimirion github login
```

This will prompt for your GitHub username and personal access token.

#### Create a New GitHub Repository

```bash
mimirion github create <name> [description] [private]
```

Example:
```bash
mimirion github create my-project "A new project created with Mimirion" true
```

## Project Structure

```
mimirion/
├── include/              # Header files
│   ├── repository.hpp    # Main repository management
│   ├── file_tracker.hpp  # File tracking and status management
│   ├── commit.hpp        # Commit creation and history management
│   ├── diff.hpp          # Diffing and patching functionality
│   ├── remote.hpp        # Remote repository management
│   ├── github_api.hpp    # GitHub API integration
│   └── utils.hpp         # Utility functions
│
├── src/                  # Implementation files
│   ├── main.cpp          # Command-line interface
│   ├── repository.cpp    # Repository implementation
│   ├── file_tracker.cpp  # File tracking implementation
│   ├── commit.cpp        # Commit functionality implementation
│   ├── diff.cpp          # Diff engine implementation
│   ├── remote.cpp        # Remote management implementation
│   ├── github_api.cpp    # GitHub API implementation
│   └── utils.cpp         # Utility functions implementation
│
├── docs/                 # Documentation (generated)
├── examples/             # Example usage and scripts
├── tests/                # Unit and integration tests
└── build/                # Build artifacts (not versioned)
```

## API Reference

### Core Classes

#### Repository

The main class representing a Mimirion repository. Key methods:

- `Repository::init(path)` - Initialize a new repository
- `Repository::status()` - Get repository status
- `Repository::add(path)` - Stage files for commit
- `Repository::commit(message)` - Create a new commit
- `Repository::createBranch(name)` - Create a new branch
- `Repository::checkout(name)` - Switch to a different branch

#### CommitManager

Handles commit creation and history management:

- `CommitManager::createCommit(message, files)` - Create a new commit
- `CommitManager::getHistory()` - Get commit history

#### FileTracker

Tracks file status and changes:

- `FileTracker::updateStatus()` - Update file status
- `FileTracker::getFiles()` - Get all tracked files
- `FileTracker::stageFile(path)` - Stage a file for commit

#### GitHubProvider

Handles GitHub API integration:

- `GitHubProvider::push(...)` - Push to GitHub repository
- `GitHubProvider::pull(...)` - Pull from GitHub repository
- `GitHubProvider::createRepository(...)` - Create a new GitHub repository

## Development

### Contributing

We welcome contributions to Mimirion! To contribute:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Commit your changes (`git commit -m 'Add amazing feature'`)
5. Push to the branch (`git push origin feature/amazing-feature`)
6. Open a Pull Request

### Coding Standards

- Follow C++17 best practices
- Use snake_case for variables and functions
- Use PascalCase for classes
- Document all public methods with Doxygen comments
- Include unit tests for new functionality

## License

[GNU General Public License v3](LICENSE)

## GitHub Integration

Mimirion integrates with GitHub to enable pushing, pulling, and repository management without leaving your workflow.

### Setting Up GitHub Access

1. Get your GitHub Personal Access Token:
   ```bash
   # Run the setup script to guide you through the process
   ./scripts/setup_github.sh
   ```

2. Add a GitHub remote to your repository:
   ```bash
   # Using the Mimirion CLI
   mimirion remote add origin https://github.com/your-username/your-repo.git
   ```

3. Push your changes to GitHub:
   ```bash
   # Push the current branch
   mimirion push origin
   ```

### Programmatic API Integration

For custom applications, Mimirion provides a C++ API for GitHub integration:

```cpp
// Set credentials
repo.setGitHubCredentials("username", "token");

// Push changes
repo.push("origin", "master");
```

For more details, see our [GitHub Integration Guide](docs/github_integration.md).

## Future Enhancements

- More advanced diff algorithm (Myers diff)
- Merge conflict resolution interface
- Interactive rebase functionality
- Pull request management via GitHub API
- GUI interface for easier interaction
- Plugin system for other remote providers (GitLab, Bitbucket, etc.)
- Performance optimizations for large repositories

## Acknowledgments

This project is for educational purposes and demonstrates the core concepts of version control systems.

## Contact

Project Link: [https://github.com/Art-Jashar/Mimirion](https://github.com/Art-Jashari/Mimirion)
