#!/bin/bash
#
# GitHub Credentials Setup Script for Mimirion VCS
#

set -e  # Exit on error

# Display banner
echo "========================================"
echo "  Mimirion VCS GitHub Credentials Setup"
echo "========================================"
echo

# Check if jq is installed for JSON processing
if ! command -v jq &> /dev/null; then
    echo "This script requires 'jq' for JSON processing."
    echo "Please install it using your package manager:"
    echo "  For Ubuntu/Debian: sudo apt install jq"
    echo "  For Fedora/RHEL: sudo dnf install jq"
    echo "  For Manjaro/Arch: sudo pacman -S jq"
    echo "  For macOS (Homebrew): brew install jq"
    exit 1
fi

# Function to create credentials file
create_credentials_file() {
    local username="$1"
    local token="$2"
    local config_dir="$HOME/.config/mimirion"
    local credentials_file="$config_dir/github_credentials"
    
    # Create config directory if it doesn't exist
    mkdir -p "$config_dir"
    
    # Create credentials file with secure permissions
    echo "username=$username" > "$credentials_file"
    echo "token=$token" >> "$credentials_file"
    chmod 600 "$credentials_file"
    
    echo "Credentials saved to: $credentials_file"
    echo "File permissions set to user-readable only (600)"
}

# Get user input
read -p "Enter your GitHub username: " github_username

# Prompt for token or offer to create one
read -p "Do you have a GitHub Personal Access Token? [y/N]: " has_token

if [[ $has_token =~ ^[Yy]$ ]]; then
    read -sp "Enter your GitHub Personal Access Token: " github_token
    echo
else
    echo "Let's create a new Personal Access Token on GitHub."
    echo "1. Open your browser and go to: https://github.com/settings/tokens"
    echo "2. Click 'Generate new token'"
    echo "3. Give it a name like 'Mimirion VCS'"
    echo "4. Select the 'repo' scope to allow repository operations"
    echo "5. Click 'Generate token'"
    echo "6. Copy the token value"
    echo
    read -sp "Paste the generated token here: " github_token
    echo
fi

# Validate token by making a test API call
echo "Validating token..."
response=$(curl -s -o /dev/null -w "%{http_code}" -H "Authorization: token $github_token" https://api.github.com/user)

if [ "$response" -eq 200 ]; then
    echo "Token validation successful!"
    
    # Get user info for confirmation
    user_info=$(curl -s -H "Authorization: token $github_token" https://api.github.com/user)
    login=$(echo "$user_info" | jq -r .login)
    name=$(echo "$user_info" | jq -r .name)
    
    echo "Authenticated as: $name ($login)"
    
    # Save credentials
    create_credentials_file "$github_username" "$github_token"
    
    echo
    echo "GitHub integration is now set up for Mimirion VCS."
    echo "You can now use the GitHub features with your projects."
else
    echo "Error: Token validation failed with response code $response."
    echo "Please check your token and try again."
    exit 1
fi
