#!/bin/bash

# HFT Bot Build Script
# Usage: ./build.sh [debug|release|clean]

BUILD_TYPE=${1:-release}
BUILD_DIR="build"
PROJECT_NAME="hft-bot"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Clean build
if [ "$BUILD_TYPE" = "clean" ]; then
    print_status "Cleaning build directory..."
    rm -rf $BUILD_DIR
    mkdir -p $BUILD_DIR
    exit 0
fi

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p $BUILD_DIR
fi

# Navigate to build directory
cd $BUILD_DIR

# Configure CMake
print_status "Configuring CMake for $BUILD_TYPE build..."
if [ "$BUILD_TYPE" = "debug" ]; then
    cmake .. -DCMAKE_BUILD_TYPE=Debug
elif [ "$BUILD_TYPE" = "release" ]; then
    cmake .. -DCMAKE_BUILD_TYPE=Release
else
    print_error "Invalid build type. Use 'debug', 'release', or 'clean'"
    exit 1
fi

# Check if CMake configuration was successful
if [ $? -ne 0 ]; then
    print_error "CMake configuration failed!"
    exit 1
fi

# Build the project
print_status "Building project..."
make -j$(nproc)

# Check if build was successful
if [ $? -eq 0 ]; then
    print_status "Build completed successfully!"
    print_status "Executable: $BUILD_DIR/$PROJECT_NAME"
    
    # Show binary size and basic info
    if [ -f "$PROJECT_NAME" ]; then
        print_status "Binary size: $(du -h $PROJECT_NAME | cut -f1)"
        print_status "Binary type: $(file $PROJECT_NAME)"
    fi
else
    print_error "Build failed!"
    exit 1
fi

# Optional: Run basic tests or checks
print_status "Build script completed!"
echo -e "\nTo run the bot:"
echo -e "  cd $BUILD_DIR && ./$PROJECT_NAME"
echo -e "\nTo debug:"
echo -e "  cd $BUILD_DIR && gdb ./$PROJECT_NAME"