#!/bin/bash
# Build script for Linux

set -e

echo "====================================="
echo "  Building Oracon Integrate for Linux"
echo "====================================="

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build-linux"
INSTALL_DIR="$PROJECT_ROOT/dist/linux"

# Parse arguments
BUILD_TYPE="${1:-Release}"
CLEAN_BUILD="${2:-false}"

echo -e "${YELLOW}Build configuration:${NC}"
echo "  Build type: $BUILD_TYPE"
echo "  Build directory: $BUILD_DIR"
echo "  Install directory: $INSTALL_DIR"
echo ""

# Clean build if requested
if [ "$CLEAN_BUILD" = "clean" ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

# Check dependencies
echo -e "${YELLOW}Checking dependencies...${NC}"

check_command() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}Error: $1 is not installed${NC}"
        exit 1
    else
        echo -e "${GREEN}✓${NC} $1 found"
    fi
}

check_command cmake
check_command g++
check_command pkg-config

# Check for optional dependencies
echo ""
echo -e "${YELLOW}Checking optional dependencies...${NC}"

if pkg-config --exists Qt6Core; then
    echo -e "${GREEN}✓${NC} Qt6 found - GUI will be built"
    HAS_QT=1
else
    echo -e "${YELLOW}!${NC} Qt6 not found - GUI will not be built"
    HAS_QT=0
fi

if pkg-config --exists libzstd; then
    echo -e "${GREEN}✓${NC} zstd found - compression support enabled"
fi

if pkg-config --exists x11; then
    echo -e "${GREEN}✓${NC} X11 found - desktop capture enabled"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo ""
echo -e "${YELLOW}Configuring...${NC}"
cmake "$PROJECT_ROOT" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DBUILD_TESTS=ON \
    -DBUILD_EXAMPLES=ON

# Build
echo ""
echo -e "${YELLOW}Building...${NC}"
cmake --build . -j$(nproc)

# Install
echo ""
echo -e "${YELLOW}Installing to $INSTALL_DIR...${NC}"
cmake --install .

# Create tarball
echo ""
echo -e "${YELLOW}Creating distribution package...${NC}"
cd "$PROJECT_ROOT/dist"
TAR_NAME="oracon-integrate-linux-$(uname -m).tar.gz"
tar -czf "$TAR_NAME" linux/

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Build completed successfully!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "Binaries location: $INSTALL_DIR/bin"
echo "Package: $PROJECT_ROOT/dist/$TAR_NAME"
echo ""
echo "To run:"
echo "  $INSTALL_DIR/bin/oracon-integrate --help"
