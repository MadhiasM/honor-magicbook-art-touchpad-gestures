#!/bin/bash

# Get version from git tags
VERSION=$(git describe --tags --always)
echo "Building gesture-daemon version: $VERSION"

set -e

# Removal of old binary
rm -f gesture-daemon
rm -f build-deb/gesture-daemon/usr/local/bin/gesture-daemon

# Compile
gcc src/gesture-daemon.c -o gesture-daemon

# move binary to build package
mv gesture-daemon build-deb/gesture-daemon/usr/local/bin/

# Build .deb packages
mkdir -p dist
dpkg-deb --build build-deb/gesture-daemon dist/gesture-daemon_${VERSION}.deb

echo "gesture-daemon_${VERSION}.deb created successfully"

