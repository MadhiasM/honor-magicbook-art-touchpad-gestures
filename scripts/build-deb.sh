#!/bin/bash

# Get version from git tags
VERSION=$(git describe --tags --always)
echo "Building gesture-daemon version: $VERSION"

set -e

# Removal of old binary
rm -f gesture-daemon
rm -f build-deb/gesture-daemon/usr/local/bin/gesture-daemon

# Compile
gcc gesture-daemon.c -o gesture-daemon

# Copy to build package
cp gesture-daemon build-deb/gesture-daemon/usr/local/bin/

# Build .deb packages
mkdir -p dist
dpkg-deb --build build-deb/gesture-daemon dist/gesture-daemon_${VERSION}.deb

# Removy binary from root
rm -f gesture-daemon

echo "gesture-daemon_${VERSION}.deb created successfully"

