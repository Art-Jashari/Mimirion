#!/bin/bash
#
# Build script for Mimirion VCS using vcpkg
#

set -e  # Exit on error

# Display banner
echo "========================================"
echo "  Mimirion VCS Build Script with vcpkg"
echo "========================================"
echo

# Check for vcpkg
if [ -z "$VCPKG_ROOT" ]; then
    if [ -d "$HOME/vcpkg" ]; then
        export VCPKG_ROOT="$HOME/vcpkg"
        echo "Found vcpkg at $VCPKG_ROOT"
    else
        echo "VCPKG_ROOT is not set and vcpkg not found in $HOME/vcpkg"
        echo "Would you like to install vcpkg now? (y/n)"
        read -r install_vcpkg
        if [[ $install_vcpkg =~ ^[Yy]$ ]]; then
            echo "Installing vcpkg to $HOME/vcpkg..."
            git clone https://github.com/microsoft/vcpkg.git "$HOME/vcpkg"
            "$HOME/vcpkg/bootstrap-vcpkg.sh"
            export VCPKG_ROOT="$HOME/vcpkg"
        else
            echo "Aborting. Please set VCPKG_ROOT to your vcpkg installation path."
            exit 1
        fi
    fi
fi

# Check if build directory exists
if [ -d "build" ]; then
    echo "Build directory exists. Do you want to clean it? (y/n)"
    read -r clean_build
    if [[ $clean_build =~ ^[Yy]$ ]]; then
        echo "Cleaning build directory..."
        rm -rf build
    fi
fi

# Create build directory if it doesn't exist
mkdir -p build
cd build || exit

# Configure with CMake
echo "Configuring project with CMake and vcpkg..."
cmake -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ..

# Build the project
echo "Building Mimirion..."
cmake --build . --config Release -- -j "$(nproc)"

echo
echo "Build completed successfully!"
echo "The executable is located at: $(pwd)/mimirion"
