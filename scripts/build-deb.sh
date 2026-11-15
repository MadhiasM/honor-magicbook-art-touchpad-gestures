#!/bin/bash

# Get version from git tags
VERSION=$(git describe --tags --always)
echo "Building gesture-daemon version: $VERSION"

set -e

mkdir -p build-deb/gesture-daemon/usr/local/bin

# Removal of old binary
rm -f gesture-daemon
rm -f build-deb/gesture-daemon/usr/local/bin/gesture-daemon
rm -f touchpadctl
rm -f build-deb/gesture-daemon/usr/local/bin/touchpadctl

# Compile
gcc src/gesture-daemon.c -o gesture-daemon
gcc -std=c11 -Wall -Wextra -Wpedantic src/touchpad-config.c \
    src/touchpadctl.c -o touchpadctl

# move binary to build package
mv gesture-daemon build-deb/gesture-daemon/usr/local/bin/
mv touchpadctl build-deb/gesture-daemon/usr/local/bin/

# Build .deb packages
mkdir -p dist
dpkg-deb --build build-deb/gesture-daemon dist/gesture-daemon_${VERSION}.deb

echo "gesture-daemon_${VERSION}.deb created successfully"

