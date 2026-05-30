#!/bin/bash
# Build script for NIMBU OS

set -e

echo "NIMBU OS Build System"
echo "====================="

# Check toolchain
if ! command -v i686-elf-gcc &> /dev/null; then
    echo "Error: i686-elf-gcc not found. Please install i686-elf-gcc cross-compiler."
    exit 1
fi

if ! command -v nasm &> /dev/null; then
    echo "Error: nasm not found. Please install NASM assembler."
    exit 1
fi

if ! command -v i686-elf-ld &> /dev/null; then
    echo "Error: i686-elf-ld not found. Please install i686-elf binutils."
    exit 1
fi

echo "Toolchain: OK"
echo ""

# Clean and build
if [ "$1" = "clean" ]; then
    echo "Cleaning build..."
    make clean
    exit 0
fi

echo "Building NIMBU OS kernel..."
make all

echo ""
echo "Build complete!"
echo "Kernel binary: build/bin/nimbu.bin"
echo ""
echo "To run: make run"
echo "To debug: make debug"
