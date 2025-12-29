#!/bin/bash
# Project Nightfall - Build All Firmware Script
# Builds firmware for all three ESP32 boards

set -e  # Exit on any error

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR="dist"
FIRMWARE_DIR="$BUILD_DIR/firmware"
VERSION_FILE="version.txt"
BUILD_DATE=$(date +%Y%m%d_%H%M%S)

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    case $status in
        "INFO")
            echo -e "${BLUE}ℹ️  $message${NC}"
            ;;
        "SUCCESS")
            echo -e "${GREEN}✅ $message${NC}"
            ;;
        "WARNING")
            echo -e "${YELLOW}⚠️  $message${NC}"
            ;;
        "ERROR")
            echo -e "${RED}❌ $message${NC}"
            ;;
        "STEP")
            echo -e "${BLUE}🔧 $message${NC}"
            ;;
    esac
}

# Function to check prerequisites
check_prerequisites() {
    print_status "STEP" "Checking prerequisites..."
    
    # Check if PlatformIO is installed
    if ! command -v pio &> /dev/null; then
        print_status "ERROR" "PlatformIO not found. Please install it first:"
        echo "  pip install platformio"
        exit 1
    fi
    
    # Check if project has platformio.ini
    if [ ! -f "platformio.ini" ]; then
        print_status "ERROR" "platformio.ini not found. Make sure you're in the project root directory."
        exit 1
    fi
    
    # Check if source files exist
    if [ ! -d "src" ]; then
        print_status "ERROR" "src directory not found."
        exit 1
    fi
    
    print_status "SUCCESS" "Prerequisites check passed"
}

# Function to clean previous builds
clean_builds() {
    print_status "STEP" "Cleaning previous builds..."
    
    if [ -d ".pio" ]; then
        pio run --target clean
        print_status "SUCCESS" "Build artifacts cleaned"
    else
        print_status "INFO" "No previous builds to clean"
    fi
}

# Function to validate code quality
validate_code_quality() {
    print_status "STEP" "Running code quality checks..."
    
    # Static analysis
    print_status "INFO" "Running static analysis..."
    if pio check --verbose; then
        print_status "SUCCESS" "Static analysis passed"
    else
        print_status "WARNING" "Static analysis found issues (continuing anyway)"
    fi
    
    # Code formatting check
    print_status "INFO" "Checking code formatting..."
    if find src/ lib/ -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror 2>/dev/null; then
        print_status "SUCCESS" "Code formatting check passed"
    else
        print_status "WARNING" "Code formatting issues found"
    fi
}

# Function to run tests
run_tests() {
    print_status "STEP" "Running test suite..."
    
    # Unit tests
    print_status "INFO" "Running unit tests..."
    if pio test --filter="unit/*" --verbose; then
        print_status "SUCCESS" "Unit tests passed"
    else
        print_status "ERROR" "Unit tests failed"
        return 1
    fi
    
    # Integration tests
    print_status "INFO" "Running integration tests..."
    if pio test --filter="integration/*" --verbose; then
        print_status "SUCCESS" "Integration tests passed"
    else
        print_status "WARNING" "Integration tests failed (continuing)"
    fi
    
    print_status "SUCCESS" "Test suite completed"
}

# Function to build firmware for specific environment
build_firmware() {
    local env_name=$1
    local board_name=$ output_name=$3
    
    print_status "INFO2
    local" "Building firmware for $board_name..."
    
    if pio run -e "$env_name" --verbose; then
        # Copy firmware to dist directory
        local source_firmware=".pio/build/$env_name/firmware.bin"
        local dest_firmware="$FIRMWARE_DIR/${output_name}_${BUILD_DATE}.bin"
        
        if [ -f "$source_firmware" ]; then
            cp "$source_firmware" "$dest_firmware"
            print_status "SUCCESS" "Firmware built: $dest_firmware"
            
            # Get firmware size
            local firmware_size=$(stat -f%z "$dest_firmware" 2>/dev/null || stat -c%s "$dest_firmware")
            print_status "INFO" "Firmware size: $(($firmware_size / 1024))KB"
            
            return 0
        else
            print_status "ERROR" "Firmware file not found: $source_firmware"
            return 1
        fi
    else
        print_status "ERROR" "Build failed for $board_name"
        return 1
    fi
}

# Function to create firmware package
create_firmware_package() {
    print_status "STEP" "Creating firmware package..."
    
    # Create firmware directory
    mkdir -p "$FIRMWARE_DIR"
    
    # Build all firmware versions
    local build_success=true
    
    # Front ESP32 (Master Controller)
    if build_firmware "front_esp32" "Front ESP32 (Master)" "front_esp32"; then
        : # Success
    else
        build_success=false
    fi
    
    # Rear ESP32 (Slave Controller)
    if build_firmware "rear_esp32" "Rear ESP32 (Slave)" "rear_esp32"; then
        : # Success
    else
        build_success=false
    fi
    
    # ESP32-CAM (Vision Module)
    if build_firmware "camera_esp32" "ESP32-CAM (Vision)" "camera_esp32"; then
        : # Success
    else
        build_success=false
    fi
    
    if [ "$build_success" = true ]; then
        # Create checksums
        print_status "INFO" "Generating checksums..."
        cd "$FIRMWARE_DIR"
        sha256sum *.bin > checksums.txt
        
        # Create manifest
        cat > manifest.json << EOF
{
    "build_info": {
        "version": "$(cat VERSION_FILE 2>/dev/null || echo '1.2.3')",
        "build_date": "$BUILD_DATE",
        "build_type": "release"
    },
    "firmware_files": [
        {
            "name": "front_esp32_${BUILD_DATE}.bin",
            "description": "Master Controller (Front ESP32)",
            "target": "front_esp32",
            "size": $(stat -f%z "front_esp32_${BUILD_DATE}.bin" 2>/dev/null || stat -c%s "front_esp32_${BUILD_DATE}.bin")
        },
        {
            "name": "rear_esp32_${BUILD_DATE}.bin",
            "description": "Slave Controller (Rear ESP32)",
            "target": "rear_esp32",
            "size": $(stat -f%z "rear_esp32_${BUILD_DATE}.bin" 2>/dev/null || stat -c%s "rear_esp32_${BUILD_DATE}.bin")
        },
        {
            "name": "camera_esp32_${BUILD_DATE}.bin",
            "description": "Vision Module (ESP32-CAM)",
            "target": "camera_esp32",
            "size": $(stat -f%z "camera_esp32_${BUILD_DATE}.bin" 2>/dev/null || stat -c%s "camera_esp32_${BUILD_DATE}.bin")
        }
    ],
    "checksums": "$(cat checksums.txt | base64)"
}
EOF
        cd - > /dev/null
        
        print_status "SUCCESS" "Firmware package created successfully"
        return 0
    else
        print_status "ERROR" "Firmware package creation failed"
        return 1
    fi
}

# Function to generate build report
generate_build_report() {
    print_status "STEP" "Generating build report..."
    
    local report_file="$BUILD_DIR/build_report_${BUILD_DATE}.txt"
    
    cat > "$report_file" << EOF
Project Nightfall - Build Report
================================
Build Date: $(date)
Build Type: Release
Version: $(cat VERSION_FILE 2>/dev/null || echo '1.2.3')

Build Summary:
--------------
$(if [ -d "$FIRMWARE_DIR" ]; then
    echo "Firmware files generated:"
    ls -la "$FIRMWARE_DIR"/*.bin 2>/dev/null | while read -r line; do
        echo "  $line"
    done
else
    echo "No firmware files generated"
fi)

Build Environment:
------------------
PlatformIO Version: $(pio --version)
Operating System: $(uname -s) $(uname -r)
Architecture: $(uname -m)
GCC Version: $(gcc --version | head -n1)

Build Statistics:
-----------------
$(echo "Total build time: $SECONDS seconds")
$(echo "Build directory: $(pwd)")
$(echo "Firmware directory: $FIRMWARE_DIR")

Next Steps:
-----------
1. Upload firmware to hardware using PlatformIO
2. Test all systems after upload
3. Verify communication between boards
4. Run deployment validation

For upload instructions, see: docs/getting-started/installation.md
EOF
    
    print_status "SUCCESS" "Build report generated: $report_file"
}

# Function to cleanup temporary files
cleanup() {
    print_status "STEP" "Cleaning up temporary files..."
    
    # Clean PlatformIO build cache
    if [ -d ".pio" ]; then
        pio run --target clean > /dev/null 2>&1 || true
    fi
    
    print_status "SUCCESS" "Cleanup completed"
}

# Main execution
main() {
    echo "🚀 Project Nightfall - Build All Firmware"
    echo "========================================="
    echo "Build Date: $(date)"
    echo "Build Version: $(cat VERSION_FILE 2>/dev/null || echo '1.2.3')"
    echo ""
    
    # Set trap for cleanup on exit
    trap cleanup EXIT
    
    # Execute build steps
    check_prerequisites
    clean_builds
    validate_code_quality
    run_tests
    
    # Create firmware package
    if create_firmware_package; then
        generate_build_report
        
        echo ""
        print_status "SUCCESS" "🎉 Build completed successfully!"
        echo ""
        echo "📦 Generated files:"
        echo "   Firmware directory: $FIRMWARE_DIR"
        echo "   Build report: $BUILD_DIR/build_report_${BUILD_DATE}.txt"
        echo ""
        echo "🔧 Next steps:"
        echo "   1. Upload firmware to hardware"
        echo "   2. Test all systems"
        echo "   3. Run deployment validation"
        echo ""
        
        # Show firmware file sizes
        if [ -d "$FIRMWARE_DIR" ]; then
            echo "📊 Firmware sizes:"
            ls -lh "$FIRMWARE_DIR"/*.bin 2>/dev/null | awk '{print "   " $9 ": " $5}' || true
        fi
        
        exit 0
    else
        print_status "ERROR" "Build failed!"
        echo ""
        echo "🔧 Troubleshooting:"
        echo "   1. Check error messages above"
        echo "   2. Verify hardware connections"
        echo "   3. Check PlatformIO configuration"
        echo "   4. Review test logs"
        echo ""
        exit 1
    fi
}

# Check if running in CI environment
if [ "$CI" = "true" ]; then
    # CI-specific settings
    export PIO_ENV=ci
    export SERIAL_DEBUG=0
fi

# Run main function
main "$@"