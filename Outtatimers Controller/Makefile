# Makefile for Outtatimers Controller PlatformIO Project
# Provides convenient commands for building, uploading, and testing

.PHONY: help build upload clean monitor test all ci

# Default target
help:
	@echo "Outtatimers Controller - Available commands:"
	@echo ""
	@echo "Build & Development:"
	@echo "  build       - Compile the project"
	@echo "  upload      - Build and upload to device"
	@echo "  clean       - Clean build files"
	@echo "  monitor     - Open serial monitor"
	@echo ""
	@echo "Testing & CI:"
	@echo "  test        - Run tests (if available)"
	@echo "  all         - Build and run tests"
	@echo "  ci          - Continuous integration build"
	@echo ""
	@echo "Project Info:"
	@echo "  deps        - Install dependencies"
	@echo "  update      - Update PlatformIO and libraries"
	@echo "  check       - Check for PlatformIO updates"

# Build the project
build:
	@echo "🔨 Building Outtatimers Controller..."
	@/usr/bin/pio run

# Build and upload to device
upload: build
	@echo "🚀 Uploading to device..."
	@/usr/bin/pio run --target upload

# Clean build files
clean:
	@echo "🧹 Cleaning build files..."
	@/usr/bin/pio run --target clean

# Open serial monitor
monitor:
	@echo "📊 Opening serial monitor..."
	@/usr/bin/pio device monitor

# Install dependencies
deps:
	@echo "📦 Installing dependencies..."
	@/usr/bin/pio pkg install

# Update PlatformIO and libraries
update:
	@echo "⬆️  Updating PlatformIO and libraries..."
	@/usr/bin/pio pkg update

# Check for updates
check:
	@echo "🔍 Checking for PlatformIO updates..."
	@/usr/bin/pio pkg outdated

# Run tests (if test environment exists)
test:
	@echo "🧪 Running tests..."
	@if /usr/bin/pio project config --json | grep -q "test"; then \
		/usr/bin/pio test; \
	else \
		echo "No test environment configured"; \
	fi

# Build and run tests
all: build test

# Continuous integration build (build only, no upload)
ci: clean build
	@echo "✅ CI build completed successfully"

# Quick development cycle - build and monitor
dev: build monitor

# Show project information
info:
	@echo "📋 Project Information:"
	@/usr/bin/pio project config
	@echo ""
	@echo "📦 Library Dependencies:"
	@cat platformio.ini | grep lib_deps