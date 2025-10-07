#!/bin/bash
# Script to build and run host-based tests for OuttatimersController
# This script sets up the ESP-IDF environment for Linux host builds and runs tests

set -e  # Exit on error

echo "=========================================="
echo "OuttatimersController Host-Based Testing"
echo "=========================================="
echo ""

# Check if IDF_PATH is set
if [ -z "$IDF_PATH" ]; then
    echo "ERROR: IDF_PATH is not set!"
    echo "Please source the ESP-IDF environment first:"
    echo "  . \$HOME/esp/esp-idf/export.sh"
    exit 1
fi

echo "Using ESP-IDF from: $IDF_PATH"
echo ""

# Set target to Linux for host builds
echo "Setting IDF_TARGET to linux..."
export IDF_TARGET=linux

# Clean previous build if requested
if [ "$1" == "clean" ]; then
    echo "Cleaning previous build..."
    rm -rf build
    echo "Clean complete."
    echo ""
fi

# Create build directory if it doesn't exist
mkdir -p build
cd build

echo "Configuring build for host testing..."
# Configure for Linux target
cmake .. -G "Unix Makefiles" -DIDF_TARGET=linux

echo ""
echo "Building tests..."
# Build the project
make -j$(nproc)

echo ""
echo "=========================================="
echo "Running tests..."
echo "=========================================="
echo ""

# Find and run the test executable
TEST_EXECUTABLE=$(find . -name "test_main.elf" -o -name "OuttatimersController.elf" | head -n 1)

if [ -z "$TEST_EXECUTABLE" ]; then
    echo "ERROR: Could not find test executable!"
    echo "Expected to find test_main.elf or OuttatimersController.elf"
    exit 1
fi

echo "Running: $TEST_EXECUTABLE"
echo ""

# Run the tests
$TEST_EXECUTABLE

echo ""
echo "=========================================="
echo "Tests completed successfully!"
echo "=========================================="