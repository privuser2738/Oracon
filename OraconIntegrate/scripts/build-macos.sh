#!/bin/bash
# Build script for macOS

set -e

echo "====================================="
echo "  Building Oracon Integrate for macOS"
echo "====================================="

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build-macos"
INSTALL_DIR="$PROJECT_ROOT/dist/macos"

# Parse arguments
BUILD_TYPE="${1:-Release}"
CLEAN_BUILD="${2:-false}"
ARCH="${3:-$(uname -m)}"  # arm64 or x86_64

echo -e "${YELLOW}Build configuration:${NC}"
echo "  Build type: $BUILD_TYPE"
echo "  Architecture: $ARCH"
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
check_command clang++

# Check for Homebrew (recommended)
if command -v brew &> /dev/null; then
    echo -e "${GREEN}✓${NC} Homebrew found"

    # Check for optional dependencies via Homebrew
    if brew list --formula | grep -q "^qt@6\$"; then
        echo -e "${GREEN}✓${NC} Qt6 found - GUI will be built"
        export Qt6_DIR="$(brew --prefix qt@6)/lib/cmake/Qt6"
    else
        echo -e "${YELLOW}!${NC} Qt6 not found - GUI will not be built"
        echo "  To install: brew install qt@6"
    fi

    if brew list --formula | grep -q "^zstd\$"; then
        echo -e "${GREEN}✓${NC} zstd found - compression support enabled"
    fi

    if brew list --formula | grep -q "^openssl@3\$"; then
        echo -e "${GREEN}✓${NC} OpenSSL found"
        export OPENSSL_ROOT_DIR="$(brew --prefix openssl@3)"
    fi
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
    -DCMAKE_OSX_ARCHITECTURES="$ARCH" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="10.15" \
    -DBUILD_TESTS=ON \
    -DBUILD_EXAMPLES=ON

# Build
echo ""
echo -e "${YELLOW}Building...${NC}"
cmake --build . -j$(sysctl -n hw.ncpu)

# Install
echo ""
echo -e "${YELLOW}Installing to $INSTALL_DIR...${NC}"
cmake --install .

# Create DMG or tarball
echo ""
echo -e "${YELLOW}Creating distribution package...${NC}"
cd "$PROJECT_ROOT/dist"

if command -v hdiutil &> /dev/null; then
    # Create DMG
    DMG_NAME="oracon-integrate-macos-$ARCH.dmg"
    hdiutil create -volname "OraconIntegrate" -srcfolder macos -ov -format UDZO "$DMG_NAME"
    echo "Package: $PROJECT_ROOT/dist/$DMG_NAME"
else
    # Create tarball
    TAR_NAME="oracon-integrate-macos-$ARCH.tar.gz"
    tar -czf "$TAR_NAME" macos/
    echo "Package: $PROJECT_ROOT/dist/$TAR_NAME"
fi

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Build completed successfully!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "Binaries location: $INSTALL_DIR/bin"
echo ""
echo "To run:"
echo "  $INSTALL_DIR/bin/oracon-integrate --help"
